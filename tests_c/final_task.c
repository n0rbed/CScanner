int calculate(int x) {
    if (x <= 0) {
        return 0;
    }
    int sum = 0;
    for (int i = 1; i <= x; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}

int main() {
    int n = 10;
    int result = calculate(n);
    return 0;
}