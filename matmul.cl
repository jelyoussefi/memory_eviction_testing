__kernel void matrixMul(__global float* A,  __global float* B,  __global float* C, 
          		  int wA, int wB)
{
   int tx = get_global_id(0); 
   int ty = get_global_id(1);
 
   float acc = 0.0;
   for (int k = 0; k < wA; ++k) {
      acc += A[ty * wA + k] * B[k * wB + tx];
   }
 
   C[ty * wA + tx] = acc;
}
