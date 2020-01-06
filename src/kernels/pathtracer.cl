__kernel void fill(__global float3 *image) {
    float delta = 0.1f;

    int gx = get_global_id(0);
    int gy = get_global_id(1);
    int gsy = get_global_size(1);

    int pos = gx * gsy + gy;

    if (pos % 3 == 0) {
        image[pos].x += delta;
    } else if (pos % 3 == 1) {
        image[pos].y += delta;
    } else if (pos % 3 == 2) {
        image[pos].z += delta;
    }
}