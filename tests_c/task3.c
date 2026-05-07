int main() {
    int result;
    result = 5 + 3 * 2;
    result = (5 + 3) * 2;
    result = a + b - c * d / e; // we know this is undefined but we just want to test the lexing and parsing
    return 0;
}