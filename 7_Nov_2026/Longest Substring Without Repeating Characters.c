
#include <stdio.h>

int lengthOfLongestSubstring(char* s) {
    int max = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        int seen[256] = {0};
        int count = 0;

        for (int j = i; s[j] != '\0'; j++) {

            if (seen[(unsigned char)s[j]] == 1) {
                break;
            }

            seen[(unsigned char)s[j]] = 1;
            count++;

            if (count > max) {
                max = count;
            }
        }
    }

    return max;
}

int main() {
    char s[1000];

    printf("Enter a string: ");
    scanf("%999s", s);

    int result = lengthOfLongestSubstring(s);

    printf("Output: %d\n", result);

    return 0;
}
