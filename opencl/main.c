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
#include"../camera.h"
#include"../world.h"
#include"../material.h"
#include"../texture.h"
#include"../objects.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stbimage.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stbimage_write.h"

#include "err_code.h"

//pick up device type from compiler command line or from
//the default type
#ifndef DEVICE
#define DEVICE CL_DEVICE_TYPE_DEFAULT
#endif

int main(){
    stbi_set_flip_vertically_on_load(1);
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

    struct Camera cam = {.camera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 0, 0}, .pos=(struct vec3){5, 5, 5}, .fov=1.5};

    initCamera(&cam, 1024,1024);

    struct World world;
    initWorld(&world);

    int white = texConst(&world, (struct vec3){1.0, 1.0, 1.0});
    int envMap = texFromFile(&world, "../environment.hdr");

    struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .normal=-1, .texture=white, .type=METAL, .emissiveColor=(struct vec3){0, 0, 0}, .ior=1.3}};

    world.materials = mats;
    world.envmap = envMap;

    float rotation[16] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float translation[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    matRotation(rotation, (struct vec3){0.5, 0.5, 0.5});
    matTranslation(translation, (struct vec3){0, 0, 2});
    struct Mesh horse = addMesh(&world, "../horse.obj", 0, NULL);

    for(int i = 0; i < 6; i++){
        float* mat = calloc(16, sizeof(float));
        float* scale = calloc(16, sizeof(float));
        matRotation(rotation, (struct vec3){0, 1.0471975512*(i), 0});
        matScale(scale, (struct vec3){i%2 ? 0.8 : 1.2, i%2 ? 0.8 : 1.2, i%2 ? 0.8 : 1.2});
        matmul4x4(mat, translation, scale);
        matmul4x4(mat, rotation, mat);
        addMeshInstance(&world, &horse, mat);
    }

    struct Hittable* objects = world.objects.data;
    int size = world.objects.size;

    enum ObjectType* objectTypes = malloc(size*sizeof(enum ObjectType));

    for(int i=0; i < size; i++){
        objectTypes[i] = world.objects.data[i].type;
    }

    struct Hittable* objPtrs[world.objects.size];
    for(int i = 0; i < world.objects.size; i++){
        objPtrs[i] = &(world.objects.data[i]);
    }

    buildBvh(world.object_data, world.matrix_data, world.tree, objPtrs, world.objects.size);

    int node_count = countNodes(world.tree);

    struct LBvh* nodes = malloc(sizeof(struct LBvh)*node_count);
    struct AABB* boxes = malloc(sizeof(struct AABB)*node_count);

    int count = -1;
    buildLBvh(nodes, boxes, world.tree, &count);

    world.boxes = boxes;
    world.lbvh_nodes = nodes;

    float* boxData = malloc(6*sizeof(float)*node_count);

    for(int i = 0; i < node_count; i++){
        boxData[6*i+0] = world.boxes[i].x0;
        boxData[6*i+1] = world.boxes[i].x1;
        boxData[6*i+2] = world.boxes[i].y0;
        boxData[6*i+3] = world.boxes[i].y1;
        boxData[6*i+4] = world.boxes[i].z0;
        boxData[6*i+5] = world.boxes[i].z1;
    }

    for(int i = 0; i < node_count; i++){
        struct LBvh node = world.lbvh_nodes[i];
    }

    size_t global;                  // global domain size

    cl_device_id     device_id;     // compute device id
    cl_context       context;       // compute context
    cl_command_queue commands;      // compute command queue
    cl_program       program;       // compute program
    cl_kernel        kernel;       // compute kernel

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
    kernel = clCreateKernel(program, "getObj", &err);
    checkError(err, "Creating kernel");

    // Create the input buffers
    //getObj(__global float* image, // WRITE
    //__global struct LBvh* lbvh, //READ
    //__global struct AABB* boxes, //READ
    //__global struct Hittable* hittables, //READ
    //__global char* hittableData, //READ
    //__global struct materialInfo* mats, //READ
    //__global struct Texture* textures, /// READ
    //__global float* textureData, //READ
    //__global float* matrixData, //READ
    //int count) //READ
    cl_mem image = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*1024*1024*3, NULL, &err);
    checkError(err, "Creating output image buffer");
    cl_mem lbvh_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct LBvh)*count, NULL, &err);
    checkError(err, "Creating lbvh buffer");
    cl_mem box_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct AABB)*count, NULL, &err);
    checkError(err, "Creating boxes buffer");
    cl_mem hittable_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct Hittable)*size, NULL, &err);
    checkError(err, "Creating hittable buffer");
    cl_mem hittableData_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, world.object_data_size, NULL, &err);
    checkError(err, "Creating hittable data buffer");
    cl_mem material_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct materialInfo)*1, NULL, &err);
    checkError(err, "Creating materials buffer");
    cl_mem texture_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(struct Texture)*world.num_textures, NULL, &err);
    checkError(err, "Creating materials buffer");
    cl_mem textureData_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*world.texture_data_size, NULL, &err);
    checkError(err, "Creating textureData buffer");
    cl_mem matrixData_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*world.matrix_data_size, NULL, &err);
    checkError(err, "Creating matrixData buffer");

    //Write stuff
    err = clEnqueueWriteBuffer(commands, lbvh_buffer, CL_TRUE, 0, sizeof(struct LBvh)*count, world.lbvh_nodes, 0, NULL, NULL);
    checkError(err, "Writing lbvh_nodes");
    err = clEnqueueWriteBuffer(commands, box_buffer, CL_TRUE, 0, sizeof(float)*count*6, boxData, 0, NULL, NULL);
    checkError(err, "Writing boxes");
    err = clEnqueueWriteBuffer(commands, hittable_buffer, CL_TRUE, 0, sizeof(struct Hittable)*world.objects.size, world.objects.data, 0, NULL, NULL);
    checkError(err, "Writing hittable objects");
    err = clEnqueueWriteBuffer(commands, hittableData_buffer, CL_TRUE, 0, world.object_data_size, world.object_data, 0, NULL, NULL);
    checkError(err, "Writing hittable object data");
    err = clEnqueueWriteBuffer(commands, material_buffer, CL_TRUE, 0, sizeof(struct materialInfo)*1, world.materials, 0, NULL, NULL);
    checkError(err, "Writing materials");
    err = clEnqueueWriteBuffer(commands, texture_buffer, CL_TRUE, 0, sizeof(struct Texture)*world.num_textures, world.textures, 0, NULL, NULL);
    checkError(err, "Writing textures");
    err = clEnqueueWriteBuffer(commands, textureData_buffer, CL_TRUE, 0, sizeof(float)*world.texture_data_size, world.texture_data, 0, NULL, NULL);
    checkError(err, "Writing texture data");
    err = clEnqueueWriteBuffer(commands, matrixData_buffer, CL_TRUE, 0, sizeof(float)*world.matrix_data_size, world.matrix_data, 0, NULL, NULL);
    checkError(err, "Writing matrix data");

    printf("%d\n", world.object_data_size);

    size = 1024*1024;

    // Set the arguments to our compute kernel

    // Create the input buffers
    //getObj(__global float* image, // WRITE
    //__global struct LBvh* lbvh, //READ
    //__global struct AABB* boxes, //READ
    //__global struct Hittable* hittables, //READ
    //__global char* hittableData, //READ
    //__global struct materialInfo* mats, //READ
    //__global float* textureData, //READ
    //__global float* matrixData, //READ
    //int count) //READ
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image);
    checkError(err, "Setting image argument 0");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &lbvh_buffer);
    checkError(err, "Setting image argument 1");
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &box_buffer);
    checkError(err, "Setting image argument 2");
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &hittable_buffer);
    checkError(err, "Setting image argument 3");
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &hittableData_buffer);
    checkError(err, "Setting image argument 4");
    err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &material_buffer);
    checkError(err, "Setting image argument 5");
    err = clSetKernelArg(kernel, 6, sizeof(cl_mem), &texture_buffer);
    checkError(err, "Setting image argument 6");
    err = clSetKernelArg(kernel, 7, sizeof(cl_mem), &textureData_buffer);
    checkError(err, "Setting image argument 7");
    err = clSetKernelArg(kernel, 8, sizeof(cl_mem), &matrixData_buffer);
    checkError(err, "Setting image argument 8");
    err = clSetKernelArg(kernel,9, sizeof(struct Camera), &cam);
    checkError(err, "Setting kernel argument 9");
    err = clSetKernelArg(kernel,10, sizeof(int), &size);
    checkError(err, "Setting kernel argument 10");

    global = 1024*1024;


    // Execute the kernel over the entire range of our 1d input data set
    // letting the OpenCL runtime choose the work-group size
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
    checkError(err, "Enqueueing kernel");


    // Wait for the commands to complete before stopping the timer
    err = clFinish(commands);
    checkError(err, "Waiting for kernel to finish");
    //checkError(err, "reading from device");
    float* img = malloc(1024*1024*3*sizeof(float));
    err = clEnqueueReadBuffer(commands, image, CL_TRUE, 0, 1024*1024*3*sizeof(float), img, 0, NULL, NULL);

    stbi_write_hdr("img.hdr", 1024, 1024, 3, img);

    free(img);

    int idx = 4;

    printf("finished\n");
    printf("%d\n", world.num_textures);
    return 0;
}