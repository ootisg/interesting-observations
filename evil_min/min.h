int min (int a, int b) {
    int x = (a - b) >> 31;
    return (a & x) | (b & ~x);
}

// We get max() for free as a bonus!
int max(int a, int b) {
    return a ^ b ^ min(a, b);
}