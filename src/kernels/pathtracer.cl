__kernel void fill(__global float *image, int width, int height, int iteration) {
    int gx = get_global_id(0);
    int gy = get_global_id(1);

    int pos = gy * width * 3 + gx * 3;

    float xFactor = (gx + iteration) / (float) width;
    float yFactor = (gy + iteration) / (float) height;
    float factor = (pos + iteration) / (float) (width * height);

    image[pos] = xFactor - floor(xFactor);
    image[pos + 1] = yFactor - floor(yFactor);
    image[pos + 2] = factor - floor(factor);
}