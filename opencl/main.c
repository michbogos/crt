#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <unistd.h>
#else
#include <CL/cl.h>
#endif
#define CL_TARGET_OPENCL_VERSION 120
#include"pcg_basic.h"
#include"../vec3.h"
#include"../matrix.h"
#include"../world.h"
#include"../material.h"
#include"../texture.h"
#include"../objects.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stbimage.h"

#include "err_code.h"

//pick up device type from compiler command line or from
//the default type
#ifndef DEVICE
#define DEVICE CL_DEVICE_TYPE_GPU
#endif

int main(){
    char * source = 0;
    long length;
    FILE * f = fopen ("kernel.cl", "rb");

    if (f)
    {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    source = malloc (length);
    if (source)
    {
        fread (source, 1, length, f);
    }
    fclose (f);
    }

    struct Texture white = texConst((struct vec3){1.0, 1.0, 1.0});

    struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .normal=NULL, .texture=&white, .type=DIELECTRIC, .emissiveColor=(struct vec3){0, 0, 0}, .ior=1.3}};

    struct World world = {.materials=mats};
    struct Texture envMap = texFromFile("../environment.hdr");

    initWorld(&world, &(envMap));

    struct Mesh horse = addMesh(&world, "../cube.obj", 0, NULL);

    struct vec3* a = calloc(1024, sizeof(struct vec3));
    struct vec3* b = calloc(1024, sizeof(struct vec3));
    struct vec3* c = calloc(1024, sizeof(struct vec3));

    struct Hittable* objects = world.objects.data;
    int size = world.objects.size;

    enum ObjectType* objectTypes = malloc(size*sizeof(enum ObjectType));

    for(int i=0; i < size; i++){
        objectTypes[i] = world.objects.data[i].type;
    }

    for(int i = 0; i < 1024; i++){
        a[i].x = 1;
        a[i].y = 0;
        a[i].z = 0;
        b[i].x = 0;
        b[i].y = 1;
        b[i].z = 0;
    }

    size_t global;                  // global domain size

    cl_device_id     device_id;     // compute device id
    cl_context       context;       // compute context
    cl_command_queue commands;      // compute command queue
    cl_program       program;       // compute program
    cl_kernel        ko_vadd;       // compute kernel

    cl_mem d_a;                     // device memory used for the input  a vector
    cl_mem d_b;                     // device memory used for the input  b vector
    cl_mem d_c;                     // device memory used for the output c vector
    cl_mem obj_buffer;
    cl_mem obj_data;

    cl_uint numPlatforms;

    // Find number of platforms
    int err = clGetPlatformIDs(0, NULL, &numPlatforms);
    checkError(err, "Finding platforms");
    if (numPlatforms == 0)
    {
        printf("Found 0 platforms!\n");
        return EXIT_FAILURE;
    }

    // Get all platforms
    cl_platform_id Platform[numPlatforms];
    err = clGetPlatformIDs(numPlatforms, Platform, NULL);
    checkError(err, "Getting platforms");

    // Secure a GPU
    for (int i = 0; i < numPlatforms; i++)
    {
        err = clGetDeviceIDs(Platform[i], DEVICE, 1, &device_id, NULL);
        if (err == CL_SUCCESS)
        {
            break;
        }
    }

    if (device_id == NULL)
        checkError(err, "Finding a device");


    // Create a compute context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    checkError(err, "Creating context");

    // Create a command queue
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    checkError(err, "Creating command queue");

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1, (const char **) &source, NULL, &err);
    checkError(err, "Creating program");

    // Build the program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n%s\n", err_code(err));
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return EXIT_FAILURE;
    }

    // Create the compute kernel from the program
    ko_vadd = clCreateKernel(program, "getObj", &err);
    checkError(err, "Creating kernel");

    // Create the input (a, b) and output (c) arrays in device memory
    d_a  = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(struct vec3) * 1024, NULL, &err);
    checkError(err, "Creating buffer d_a");

    d_b  = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(struct vec3) * 1024, NULL, &err);
    checkError(err, "Creating buffer d_b");

    d_c  = clCreateBuffer(context,  CL_MEM_WRITE_ONLY, sizeof(struct vec3) * 1024, NULL, &err);
    checkError(err, "Creating buffer d_c");

    obj_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct Hittable)*size, NULL, &err);
    checkError(err, "Creating object buffer");

    obj_data = clCreateBuffer(context, CL_MEM_READ_ONLY, world.object_data_size, NULL, &err);
    checkError(err, "Creating object data buffer");

    // Write a and b vectors into compute device memory
    err = clEnqueueWriteBuffer(commands, d_a, CL_TRUE, 0, sizeof(struct vec3) * 1024, a, 0, NULL, NULL);
    checkError(err, "Copying h_a to device at d_a");

    err = clEnqueueWriteBuffer(commands, d_b, CL_TRUE, 0, sizeof(struct vec3) * 1024, b, 0, NULL, NULL);
    checkError(err, "Copying h_b to device at d_b");

    err = clEnqueueWriteBuffer(commands, obj_buffer, CL_TRUE, 0, sizeof(struct Hittable)*size, world.objects.data, 0, NULL, NULL);
    checkError(err, "Writing objects");

    err = clEnqueueWriteBuffer(commands, obj_data, CL_TRUE, 0, world.object_data_size, world.object_data, 0, NULL, NULL);
    checkError(err, "Writing object data");
    // Set the arguments to our compute kernel
    err  = clSetKernelArg(ko_vadd, 0, sizeof(cl_mem), &obj_buffer);
    checkError(err, "Setting kernel arguments 0");
    err = clSetKernelArg(ko_vadd, 1, sizeof(cl_mem), &obj_data);
    checkError(err, "Setting kernel arguments 1");
    err |= clSetKernelArg(ko_vadd, 2, sizeof(int), &size);
    checkError(err, "Setting kernel arguments 2");


    // Execute the kernel over the entire range of our 1d input data set
    // letting the OpenCL runtime choose the work-group size
    global = size;
    err = clEnqueueNDRangeKernel(commands, ko_vadd, 1, NULL, &global, NULL, 0, NULL, NULL);
    checkError(err, "Enqueueing kernel");

    // Wait for the commands to complete before stopping the timer
    err = clFinish(commands);
    checkError(err, "Waiting for kernel to finish");

    printf("finished");
    printf("%d", size);
    return 0;
}