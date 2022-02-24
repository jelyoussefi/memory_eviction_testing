__kernel void vadd(__global const float* a, __global const float* b,  __global float* c)
{
    unsigned long gid = get_global_id(0);
    c[gid] = a[gid] + b[gid];
}
