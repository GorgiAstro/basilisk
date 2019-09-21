#ifndef GROUP_SIZE
#define GROUP_SIZE (64)
#endif

#ifndef GROUP_COUNT
#define GROUP_COUNT (64)
#endif

#ifndef OPERATIONS
#define OPERATIONS (1)
#endif

#ifndef CONFIG_USE_DOUBLE
#define CONFIG_USE_DOUBLE (0)
#endif

#if CONFIG_USE_DOUBLE

#if defined(cl_khr_fp64)  // Khronos extension available?
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define DOUBLE_SUPPORT_AVAILABLE
#elif defined(cl_amd_fp64)  // AMD extension available?
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define DOUBLE_SUPPORT_AVAILABLE
#endif

#endif // CONFIG_USE_DOUBLE

#include </Users/kenneall/Documents/Software/BasiliskFork/src/simulation/dynamics/openGLCLSrp/resources/cl_kernels/utils.cl>
////////////////////////////////////////////////////////////////////////////////////////////////////

#define GROUP_SIZE_2 (GROUP_SIZE + GROUP_SIZE)
#define GROUP_SIZE_3 (GROUP_SIZE_2 + GROUP_SIZE)

////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define LOAD_GLOBAL_F3(s, i) \
vload3((size_t)(i), (__global const float*)(s))

#define LOAD_GLOBAL_F4(s, i) \
vload4((size_t)(i), (__global const float*)(s))

#define STORE_GLOBAL_F3(s, i, v) \
vstore3((v), (size_t)(i), (__global float*)(s))

#define STORE_GLOBAL_F4(s, i, v) \
vstore4((v), (size_t)(i), (__global float*)(s))

#define LOAD_GLOBAL_F16(s, i) \
vload16((size_t)(i), (__global const float*)(s))

#define LOAD_LOCAL_F1(s, i) \
((__local const float*)(s))[(size_t)(i)]

#define STORE_LOCAL_F1(s, i, v) \
((__local float*)(s))[(size_t)(i)] = (v)

#define LOAD_LOCAL_F3(s, i) \
(float3)( (LOAD_LOCAL_F1(s, i)), \
(LOAD_LOCAL_F1(s, i + GROUP_SIZE  )), \
(LOAD_LOCAL_F1(s, i + GROUP_SIZE_2)))

#define LOAD_LOCAL_F4(s, i) \
(float4)( (LOAD_LOCAL_F1(s, i     )), \
(LOAD_LOCAL_F1(s, i + GROUP_SIZE  )), \
(LOAD_LOCAL_F1(s, i + GROUP_SIZE_2)), \
(LOAD_LOCAL_F1(s, i + GROUP_SIZE_3)))

#define STORE_LOCAL_F3(s, i, v) \
STORE_LOCAL_F1(s, i,                (v)[0]); \
STORE_LOCAL_F1(s, i + GROUP_SIZE,   (v)[1]); \
STORE_LOCAL_F1(s, i + GROUP_SIZE_2, (v)[2])

#define STORE_LOCAL_F4(s, i, v) \
STORE_LOCAL_F1(s, i,                (v)[0]); \
STORE_LOCAL_F1(s, i + GROUP_SIZE,   (v)[1]); \
STORE_LOCAL_F1(s, i + GROUP_SIZE_2, (v)[2]); \
STORE_LOCAL_F1(s, i + GROUP_SIZE_3, (v)[3])

#define ACCUM_LOCAL_F3(s, i, j) \
{ \
float3 x = LOAD_LOCAL_F3(s, i); \
float3 y = LOAD_LOCAL_F3(s, j); \
float3 xy = x + y; \
STORE_LOCAL_F3(s, i, xy); \
}

#define ACCUM_LOCAL_F4(s, i, j) \
{ \
float4 x = LOAD_LOCAL_F4(s, i); \
float4 y = LOAD_LOCAL_F4(s, j); \
float4 xy = x + y; \
STORE_LOCAL_F4(s, i, xy); \
}

#define DEBUG
#define SOLAR_FLUX_EARTH 1360.8 // [W/m^2] solar flux at earth
#define SPEED_OF_LIGHT 299792458.0  // [m/s] speed of light

#define PRINT_I2(name, value) printf("%s: %d, %d\n", (name), (value)[0], (value)[1])

#define PRINT_F2(name, value) printf("%s: %f, %f\n", (name), (value)[0], (value)[1])

#define PRINT_I(name, value) printf("%s: %d\n", (name), (value))

#define PRINT_F(name, value) printf("%s: %f\n", (name), (value))

#define PRINT_F3(name, v) \
printf("%s: %f, %f, %f\n", (name), (v)[0], (v)[1], (v)[2])

#define PRINT_F4(name, v) \
printf("%s: %f, %f, %f, %f\n", (name), (v)[0], (v)[1], (v)[2], (v)[3])

#define PRINT_M4(name, mat) \
printf("%s:\n", (name)); printf("%f, %f, %f, %f\n", (mat).m0[0], (mat).m0[1], (mat).m0[2], (mat).m0[3]); printf("%f, %f, %f, %f\n", (mat).m1[0], (mat).m1[1], (mat).m1[2], (mat).m1[3]); printf("%f, %f, %f, %f\n", (mat).m2[0], (mat).m2[1], (mat).m2[2], (mat).m2[3]); printf("%f, %f, %f, %f\n", (mat).m3[0], (mat).m3[1], (mat).m3[2], (mat).m3[3])

//#if defined(DOUBLE_SUPPORT_AVAILABLE)
//
//#define PRINT_D(name, value) printf("%s: %.12f\n", (name), (value))
//
//#define PRINT_D3(name, v) \
//printf("%s: %.12f, %.12f, %.12f\n", (name), (v)[0], (v)[1], (v)[2])
//
//#define PRINT_D4(name, v) \
//printf("%s: %.12f, %.12f, %.12f, %.12f\n", (name), (v)[0], (v)[1], (v)[2], (v)[3])
//
//#define LOAD_GLOBAL_D3(s, i) \
//vload3((size_t)(i), (__global const double*)(s))
//
//#define LOAD_GLOBAL_D4(s, i) \
//vload4((size_t)(i), (__global const double*)(s))
//
//#define STORE_GLOBAL_D4(s, i, v) \
//vstore4((v), (size_t)(i), (__global double*)(s))
//
//#define LOAD_LOCAL_D1(s, i) \
//((__local const double*)(s))[(size_t)(i)]
//
//#define STORE_LOCAL_D1(s, i, v) \
//((__local double*)(s))[(size_t)(i)] = (v)
//
//#define LOAD_LOCAL_D4(s, i) \
//(double4)( (LOAD_LOCAL_D1(s, i    )), \
//(LOAD_LOCAL_D1(s, i + GROUP_SIZE  )), \
//(LOAD_LOCAL_D1(s, i + GROUP_SIZE_2)), \
//(LOAD_LOCAL_D1(s, i + GROUP_SIZE_3)))
//
//#define STORE_LOCAL_D3(s, i, v) \
//STORE_LOCAL_D1(s, i,                (v)[0]); \
//STORE_LOCAL_D1(s, i + GROUP_SIZE,   (v)[1]); \
//STORE_LOCAL_D1(s, i + GROUP_SIZE_2, (v)[2])
//
//#define STORE_LOCAL_D4(s, i, v) \
//STORE_LOCAL_D1(s, i,                (v)[0]); \
//STORE_LOCAL_D1(s, i + GROUP_SIZE,   (v)[1]); \
//STORE_LOCAL_D1(s, i + GROUP_SIZE_2, (v)[2]); \
//STORE_LOCAL_D1(s, i + GROUP_SIZE_3, (v)[3])
//
//#define ACCUM_LOCAL_D4(s, i, j) \
//{ \
//double4 x = LOAD_LOCAL_D4(s, i); \
//double4 y = LOAD_LOCAL_D4(s, j); \
//double4 xy = x + y; \
//STORE_LOCAL_D4(s, i, xy); \
//}
//__kernel void
//normalsSrpParallelReduce(read_only image2d_t normalsMap,
//                         read_only image2d_t pixelPosMap,
//                         __local double4 *shared,
//                         __global double4 *output,
//                         __global float3 *sHat_B_in,
//                         const float rho_d,
//                         const float rho_s,
//                         const float pixelArea,
////                                  const double near,
////                                  const double far,
//                         const unsigned int texWidth,
//                         const unsigned int texHeight,
////                         const unsigned int primitiveOffset,
//                         const unsigned int textureSize
//                         )
//{
//    const double4 zeroVec4d = (double4)(0.0, 0.0, 0.0, 0.0);
//    const float4 zeroVec4f = (float4)(0.0, 0.0, 0.0, 0.0);
////    const double3 zeroVec3d = (double3)(0.0, 0.0, 0.0);
//    
//    const unsigned int group_id = get_global_id(0) / get_local_size(0);
//    const unsigned int group_size = GROUP_SIZE;
//    const unsigned int group_stride = 2 * group_size;
//    const size_t local_stride = group_stride * GROUP_COUNT;
////    const double P_sun = SOLAR_FLUX_EARTH/SPEED_OF_LIGHT;
//    
//    const float rho_a = 1.0 - rho_d - rho_s;
////    double3 sHat_B = double3(sHat_B_in[0], sHat_B_in[1], sHat_B_in[2]);
//    float4 sHat_B = (float4)(sHat_B_in[0].x, sHat_B_in[0].y, sHat_B_in[0].z, 0.0);
//   
////    float16 t_mat = LOAD_GLOBAL_F16(transform, 0);
////    float16 c_mat = LOAD_GLOBAL_F16(cameraMat, 0);
////    float16 p_mat = LOAD_GLOBAL_F16(projMat, 0);
////    float16 SB_flat = LOAD_GLOBAL_F16(SB_in, 0);
//    
//    const size_t local_id = get_local_id(0);
//    STORE_LOCAL_D4(shared, local_id, zeroVec4d);
//    size_t i = group_id * group_stride + local_id;
//    int printGroup = -1;
//    if (i == 0)
//    {
////        PRINT_F("area", pixelArea);
////        PRINT_F("rho_a", rho_a);
////        PRINT_F("rhod", rho_d);
////        PRINT_F("rhos", rho_s);
////        PRINT_I("texWidth", texWidth);
////        PRINT_I("texHeight", texHeight);
////        PRINT_I("textureSize", textureSize);
////        PRINT_F4("sHatB", sHat_B);
////        PRINT_I("localid", local_id);
////        PRINT_I("get_local_size(0)", get_local_size(0));
////        PRINT_I("get_global_id(0)", get_global_id(0));
////        PRINT_I("local_stride", local_stride);
////        PRINT_I("group_stride", group_stride);
//    }
//        while (i < textureSize)
//        {
//            ////////////////
//            // First pixel//
//            ////////////////
//            int y = i / texWidth;
//            int x = i % texWidth;
//            int2 coords = (int2)(x, y);
//            float4 nHat_B = read_imagef(normalsMap, coords);
//            float4 r_B = read_imagef(pixelPosMap, coords);
//            float4 force_f = zeroVec4f;
//            float4 torque_f = zeroVec4f;
////            if (i < 100) {
////                PRINT_I("x", x);
////                PRINT_I("y", y);
////                PRINT_I2("coords", coords);
////            }
////            PRINT_F4("nHatB", nHat_B);
//            if (nHat_B[3] > 0) {
////                PRINT_F4("nHatB", nHat_B);
////                PRINT_F4("sHatB", sHat_B);
//                float cosTheta = dot(nHat_B, sHat_B);
////                PRINT_F("cosTheta", cosTheta);
////                PRINT_F("area", pixelArea);
////                PRINT_I2("coords", coords);
//                force_f = -pixelArea*(rho_a*sHat_B + 2.0f*rho_s*cosTheta*nHat_B + rho_d*(sHat_B + (2.0f/3.0f)*nHat_B));
////                force_f = (float4)(1.0, 2.0, 3.0, 4.0);
////                if (group_id == printGroup) {
////                    PRINT_F4("forcef for i", force_f);
////                    PRINT_F4("nHatB", nHat_B);
////                    PRINT_F("cosTheta", cosTheta);
////                }
//                
//                torque_f = cross(r_B, force_f);
//            }
//            
//            /////////////////
//            // Second facet//
//            /////////////////
//            unsigned int secondPixelIdx = i + group_size;
////            if (group_id == printGroup) PRINT_I("secondPixelIdx", secondPixelIdx);
//
////            PRINT_I("secondPixelIdx", secondPixelIdx);
////            PRINT_I("secondPixelIdx", secondPixelIdx);
////            double3 force_1 = zeroVec3;
////            double3 torque_1 = zeroVec3;
//            float4 force_f_1 = zeroVec4f;
//            float4 torque_f_1 = zeroVec4f;
//            // If the mesh size is smaller than the group_size then we have to stop
//            // trying to compute the second facet in the parallel reduce because there
//            // will be no more facets in the mesh.
//            if (secondPixelIdx < textureSize)
//            {
////                printf("group_id: %d, i: %d \n", group_id, secondPixelIdx);
//
////                PRINT_I("secondPixelIdx", secondPixelIdx);
//                int x_1 = secondPixelIdx % texWidth;
//                int y_1 = secondPixelIdx / texWidth;
//                int2 coords_1 = (int2)(x_1, y_1);
////                if (secondPixelIdx < 100) PRINT_I2("coords1", coords_1);
//                float4 nHat_B_1 = read_imagef(normalsMap, coords_1);
//                float4 r_B_1 = read_imagef(pixelPosMap, coords_1);
////                PRINT_F4("nHatB1", nHat_B_1);
//
//                if (nHat_B_1[3] > 0) {
//                    PRINT_F("area", pixelArea);
//                    PRINT_I2("coords1", coords_1);
//                    float cosTheta_1 = dot(nHat_B_1, sHat_B);
//                    force_f_1 = -pixelArea*(rho_a*sHat_B + 2.0f*rho_s*cosTheta_1*nHat_B_1 + rho_d*(sHat_B + (2.0f/3.0f)*nHat_B_1));
////                    force_f_1 = (float4)(1.0, 2.0, 3.0, 4.0);
//                    torque_f_1 = cross(r_B_1, force_f_1);
////                    if (group_id == printGroup) PRINT_F4("force_f_1 for secondPixelIdx", force_f_1);
////                    PRINT_F3("forcef1", force_f_1);
//                }
//            }
//            
//            double3 force_1 = (double3)(force_f_1.x, force_f_1.y, force_f_1.z);
//            double3 torque_1 = (double3)(torque_f_1.x, torque_f_1.y, torque_f_1.z);
//            double3 force = (double3)(force_f.x, force_f.y, force_f.z);
//            double3 torque = (double3)(torque_f.x, torque_f.y, torque_f.z);
//            
//            // Sum results together
//            double4 s = LOAD_LOCAL_D4(shared, local_id);
////            if (group_id == printGroup) {
////                printf("group_id %d, local_id %d: s: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, s[0], s[1], s[2], s[3]);
////            }
//            double4 tmpForce = (double4)(force[0], force[1], force[2], 0.0);
////            PRINT_D4("tmpForce", tmpForce);
//            double4 tmpForce_1 = (double4)(force_1[0], force_1[1], force_1[2], 0.0);
////            PRINT_D4("tmpForce1", tmpForce_1);
//            STORE_LOCAL_D4(shared, local_id, (tmpForce_1 + tmpForce + s));
//            
//            double4 s_post_sum = LOAD_LOCAL_D4(shared, local_id);
////            if (group_id == printGroup) {
////                printf("group_id %d, local_id %d: s_post_sum: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, s_post_sum[0], s_post_sum[1], s_post_sum[2], s_post_sum[3]);
////            }
//            i += local_stride;
//        }
//
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 512)
//        if (local_id < 256) { ACCUM_LOCAL_D4(shared, local_id, local_id + 256);}
//            
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 256)
//        if (local_id < 128) { ACCUM_LOCAL_D4(shared, local_id, local_id + 128);}
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 128)
//        if (local_id <  64) { ACCUM_LOCAL_D4(shared, local_id, local_id +  64);}
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 64)
//        if (local_id <  32) { ACCUM_LOCAL_D4(shared, local_id, local_id +  32);
////            double x_1 = LOAD_LOCAL_D1(shared, local_id    );
////            double y_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE  );
////            double z_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE_2);
////            double w_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE_3);
////            PRINT_D("x_1", x_1);
////            PRINT_D("y_1", y_1);
////            PRINT_D("z_1", z_1);
////            PRINT_D("w_1", w_1);
////            PRINT_I("localid", local_id);
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("64 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//        
//        
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 32)
//        if (local_id <  16) { ACCUM_LOCAL_D4(shared, local_id, local_id +  16);
//            
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("32 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 16)
//        if (local_id <   8) { ACCUM_LOCAL_D4(shared, local_id, local_id +   8);
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("16 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 8)
//        if (local_id <   4) { ACCUM_LOCAL_D4(shared, local_id, local_id +   4);
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("8 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 4)
//        if (local_id <   2) { ACCUM_LOCAL_D4(shared, local_id, local_id +   2);
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("4 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//#endif
//        
//        barrier(CLK_LOCAL_MEM_FENCE);
//#if (GROUP_SIZE >= 2)
//        if (local_id <   1) { ACCUM_LOCAL_D4(shared, local_id, local_id +   1);
////            if (group_id == printGroup) {
////                double4 v = LOAD_LOCAL_D4(shared, local_id);
////                printf("2 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////            }
//        }
//#endif
////            if (local_id > 0)
////            {
////                STORE_LOCAL_D4(shared, local_id, zeroVec4d);
//////                STORE_LOCAL_D4(shared, 64, zeroVec4d);
//////                STORE_LOCAL_D4(shared, local_id+64, zeroVec4d);
////                
////            }
////            double4 v = LOAD_LOCAL_D4(shared, local_id);
////            printf("End Loop group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
//    
//    barrier(CLK_LOCAL_MEM_FENCE);
//    if (get_local_id(0) == 0)
//    {
//        double4 v = LOAD_LOCAL_D4(shared, 0);
//        STORE_GLOBAL_D4(output, group_id, v);
//        #ifdef DEBUG
////        if (group_id == 0) {
//            //printf("LAST group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
////        }
////            printf("LAST group_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , v[0], v[1], v[2], v[3]);
////            printf("[%.6e,%.6e,%.6e,%.6e] \n" , v[0], v[1], v[2], v[3]);
//        #endif
//    }
//}
//
//#endif // #if defined(DOUBLE_SUPPORT_AVAILABLE)

__kernel void
normalsSrpParallelReduceFloat(read_only image2d_t normalsMap,
                              read_only image2d_t pixelPosMap,
                              read_only image2d_t materialMap,
                              __local float4 *shared_force,
                              __local float4 *shared_torque,
                              __global float4 *output_force,
                              __global float4 *output_torque,
                              __global float3 *sHat_B_in,
                              const float pixelArea,
                              const unsigned int texWidth,
                              const unsigned int texHeight,
                              const unsigned int textureSize
                              )
{
    //    const double4 zeroVec4d = (double4)(0.0, 0.0, 0.0, 0.0);
    const float4 zeroVec4f = (float4)(0.0, 0.0, 0.0, 0.0);
    //    const double3 zeroVec3d = (double3)(0.0, 0.0, 0.0);
    
    const unsigned int group_id = get_global_id(0) / get_local_size(0);
    const unsigned int group_size = GROUP_SIZE;
    const unsigned int group_stride = 2 * group_size;
    const size_t local_stride = group_stride * GROUP_COUNT;

    float4 sHat_B = (float4)(sHat_B_in[0].x, sHat_B_in[0].y, sHat_B_in[0].z, 0.0);
    //    float16 t_mat = LOAD_GLOBAL_F16(transform, 0);
    //    float16 c_mat = LOAD_GLOBAL_F16(cameraMat, 0);
    //    float16 p_mat = LOAD_GLOBAL_F16(projMat, 0);
    //    float16 SB_flat = LOAD_GLOBAL_F16(SB_in, 0);
    
    const size_t local_id = get_local_id(0);
    STORE_LOCAL_F4(shared_force, local_id, zeroVec4f);
    STORE_LOCAL_F4(shared_torque, local_id, zeroVec4f);
    size_t i = group_id * group_stride + local_id;
    int printGroup = -1;
    if (i == 0)
    {
        //        PRINT_F("area", pixelArea);
        //        PRINT_F("rho_a", rho_a);
        //        PRINT_F("rhod", rho_d);
        //        PRINT_F("rhos", rho_s);
        //        PRINT_I("texWidth", texWidth);
        //        PRINT_I("texHeight", texHeight);
        //        PRINT_I("textureSize", textureSize);
        //        PRINT_F4("sHatB", sHat_B);
        //        PRINT_I("localid", local_id);
        //        PRINT_I("get_local_size(0)", get_local_size(0));
        //        PRINT_I("get_global_id(0)", get_global_id(0));
        //        PRINT_I("local_stride", local_stride);
        //        PRINT_I("group_stride", group_stride);
    }
    while (i < textureSize)
    {
        ////////////////
        // First pixel//
        ////////////////
        int y = i / texWidth;
        int x = i % texWidth;
        int2 coords = (int2)(x, y);
        float4 nHat_B = read_imagef(normalsMap, coords);
//        if (nHat_B[3] > 0.0) printf("nHat_B: %.3f, %.3f, %.3f, %.3f \n", nHat_B[0], nHat_B[1], nHat_B[2], nHat_B[3]);
        float4 force_f = zeroVec4f;
        float4 torque_f = zeroVec4f;
        
        if (nHat_B[3] > 0) {
            float4 r_B = read_imagef(pixelPosMap, coords);
            //        if (r_B[3] > 0.0) printf("r_B: %.3f, %.3f, %.3f, %.3f \n", r_B[0], r_B[1], r_B[2], r_B[3]);
            float4 mat_coeffs = read_imagef(materialMap, coords);
//                    printf("mat_coeffs: %.3f, %.3f, %.3f, %.3f \n", mat_coeffs.x,
//                                                    mat_coeffs.y,
//                                                    mat_coeffs.z,
//                                                    mat_coeffs.w);
            float rho_s = mat_coeffs.x;
            float rho_d = mat_coeffs.y;
            float rho_a = mat_coeffs.z;

//            force_f = -pixelArea*(rho_a*sHat_B + 2.f*rho_s*nHat_B + rho_d*(sHat_B + (2.f/3.f)*nHat_B));
            float cosTheta = dot(nHat_B, sHat_B);
            force_f = -pixelArea*(rho_a*sHat_B + 2.f*rho_s*cosTheta*nHat_B + rho_d*(sHat_B + (2.f/3.f)*nHat_B));
//            force_f.x = 1.0; force_f.y = 0.0; force_f.z = 0.0; force_f.w = 0.0;
            torque_f = cross(r_B, force_f);
//            printf("force_f:%d, %.3f, %.3f, %.3f, %.3f \n", local_id, force_f[0], force_f[1], force_f[2], force_f[3]);
//            printf("torque_f: %.3f, %.3f, %.3f, %.3f \n", torque_f[0], torque_f[1], torque_f[2], torque_f[3]);
        }
        
        /////////////////
        // Second facet//
        /////////////////
        unsigned int secondPixelIdx = i + group_size;
        float4 force_f_1 = zeroVec4f;
        float4 torque_f_1 = zeroVec4f;
        // If the mesh size is smaller than the group_size then we have to stop
        // trying to compute the second facet in the parallel reduction because there
        // will be no more facets in the mesh.
        if (secondPixelIdx < textureSize)
        {
            int x_1 = secondPixelIdx % texWidth;
            int y_1 = secondPixelIdx / texWidth;
            int2 coords_1 = (int2)(x_1, y_1);
            float4 nHat_B_1 = read_imagef(normalsMap, coords_1);
            
            if (nHat_B_1[3] > 0) {
                float4 r_B_1 = read_imagef(pixelPosMap, coords_1);
                float4 mat_coeffs_1 = read_imagef(materialMap, coords_1);
                float rho_s_1 = mat_coeffs_1.x;
                float rho_d_1 = mat_coeffs_1.y;
                float rho_a_1 = mat_coeffs_1.z;
                float cosTheta_1 = dot(nHat_B_1, sHat_B);

//                force_f_1 = -pixelArea*(rho_a_1*sHat_B + 2.f*rho_s_1*nHat_B_1 + rho_d_1*(sHat_B + (2.f/3.f)*nHat_B_1));
                force_f_1 = -pixelArea*(rho_a_1*sHat_B + 2.f*rho_s_1*cosTheta_1*nHat_B_1 + rho_d_1*(sHat_B + (2.f/3.f)*nHat_B_1));
//                force_f_1.x = 1.0; force_f_1.y = 0.0; force_f_1.z = 0.0; force_f_1.w = 0.0;
                torque_f_1 = cross(r_B_1, force_f_1);
                
//                if (get_global_id(0) == 53) {
////                    printf("rho_a_1: %.3f \n", rho_a_1);
////                    printf("rho_s_1: %.3f \n", rho_s_1);
////                    printf("rho_d_1: %.3f \n", rho_d_1);
//                
//                printf("force_f_1: %.12f, %.12f, %.12f, %.12f \n", force_f_1[0], force_f_1[1], force_f_1[2], force_f_1[3]);
//                printf("nHat_B_1: %.12f, %.12f, %.12f, %.12f \n", nHat_B_1[0], nHat_B_1[1], nHat_B_1[2], nHat_B_1[3]);
//                printf("sHat_B: %.12f, %.12f, %.12f, %.12f \n", sHat_B[0], sHat_B[1], sHat_B[2], sHat_B[3]);
////                    printf("r_B_1: %.3f, %.3f, %.3f, %.3f \n", r_B_1[0], r_B_1[1], r_B_1[2], r_B_1[3]);
//                printf("mat_coeffs_1: %.3f, %.3f, %.3f, %.3f \n", mat_coeffs_1[0],
//                                                                   mat_coeffs_1[1],
//                                                                   mat_coeffs_1[2],
//                                                                   mat_coeffs_1[3]);
//                printf("pixelArea: %.12f \n", pixelArea);
////                    printf("force_f_1: %.12f, %.12f, %.12f, %.12f \n", force_f_1[0], force_f_1[1], force_f_1[2], force_f_1[3]);
////                printf("torque_f_1: %.3f, %.3f, %.3f, %.3f \n", torque_f_1[0], torque_f_1[1], torque_f_1[2], torque_f_1[3]);
//                }
                
            }
        }
        
        // Sum results together
        float4 s_force = LOAD_LOCAL_F4(shared_force, local_id);
        float4 s_torque = LOAD_LOCAL_F4(shared_torque, local_id);
        //            if (group_id == printGroup) {
        //                printf("group_id %d, local_id %d: s: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, s[0], s[1], s[2], s[3]);
        //            }
        float4 tmpForce = (float4)(force_f[0], force_f[1], force_f[2], 0.0);
        float4 tmpTorque = (float4)(torque_f[0], torque_f[1], torque_f[2], 0.0);
        //            PRINT_D4("tmpForce", tmpForce);
        float4 tmpForce_1 = (float4)(force_f_1[0], force_f_1[1], force_f_1[2], 0.0);
        float4 tmpTorque_1 = (float4)(torque_f_1[0], torque_f_1[1], torque_f_1[2], 0.0);
        //            PRINT_D4("tmpForce1", tmpForce_1);
        STORE_LOCAL_F4(shared_force, local_id, (tmpForce_1 + tmpForce + s_force));
        STORE_LOCAL_F4(shared_torque, local_id, (tmpTorque_1 + tmpTorque + s_torque));
//        float4 s_post_sum = LOAD_LOCAL_F4(shared, local_id);
        //            if (group_id == printGroup) {
        //                printf("group_id %d, local_id %d: s_post_sum: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, s_post_sum[0], s_post_sum[1], s_post_sum[2], s_post_sum[3]);
        //            }
        i += local_stride;
    }
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 512)
    if (local_id < 256) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id + 256);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id + 256);
    }
    
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 256)
    if (local_id < 128) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id + 128);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id + 128);
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 128)
    if (local_id <  64) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +  64);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +  64);
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 64)
    if (local_id <  32) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +  32);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +  32);
        //            double x_1 = LOAD_LOCAL_D1(shared, local_id    );
        //            double y_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE  );
        //            double z_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE_2);
        //            double w_1 = LOAD_LOCAL_D1(shared, local_id + GROUP_SIZE_3);
        //            PRINT_D("x_1", x_1);
        //            PRINT_D("y_1", y_1);
        //            PRINT_D("z_1", z_1);
        //            PRINT_D("w_1", w_1);
        //            PRINT_I("localid", local_id);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("64 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
    
    
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 32)
    if (local_id <  16) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +  16);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +  16);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("32 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 16)
    if (local_id <   8) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +   8);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +   8);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("16 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 8)
    if (local_id <   4) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +   4);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +   4);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("8 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 4)
    if (local_id <   2) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +   2);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +   2);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("4 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
#endif
    
    barrier(CLK_LOCAL_MEM_FENCE);
#if (GROUP_SIZE >= 2)
    if (local_id <   1) {
        ACCUM_LOCAL_F4(shared_force, local_id, local_id +   1);
        ACCUM_LOCAL_F4(shared_torque, local_id, local_id +   1);
        //            if (group_id == printGroup) {
        //                double4 v = LOAD_LOCAL_D4(shared, local_id);
        //                printf("2 group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //            }
    }
#endif
    //            if (local_id > 0)
    //            {
    //                STORE_LOCAL_D4(shared, local_id, zeroVec4d);
    ////                STORE_LOCAL_D4(shared, 64, zeroVec4d);
    ////                STORE_LOCAL_D4(shared, local_id+64, zeroVec4d);
    //
    //            }
    //            double4 v = LOAD_LOCAL_D4(shared, local_id);
    //            printf("End Loop group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
    
    barrier(CLK_LOCAL_MEM_FENCE);
    if (get_local_id(0) == 0)
    {
        float4 v_force = LOAD_LOCAL_F4(shared_force, 0);
        float4 v_torque = LOAD_LOCAL_F4(shared_torque, 0);
        STORE_GLOBAL_F4(output_force, group_id, v_force);
        STORE_GLOBAL_F4(output_torque, group_id, v_torque);
#ifdef DEBUG
        //        if (group_id == 0) {
        //printf("LAST group_id %d, local_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , local_id, v[0], v[1], v[2], v[3]);
        //        }
        //            printf("LAST group_id %d: v: [%.6e,%.6e,%.6e,%.6e] \n", group_id , v[0], v[1], v[2], v[3]);
        //            printf("[%.6e,%.6e,%.6e,%.6e] \n" , v[0], v[1], v[2], v[3]);
#endif
    }
}
