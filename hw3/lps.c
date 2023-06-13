#include <stdio.h>

int strlen(char *str)
{
	int t = 0;
	while (str[t] != '\0')
		++t;
	return t;
}

int lps(char* str)
{
	int n = strlen(str); // calculating size of string
	if (n <= 1) // in this case there is only one letter
		return n; 
	int maxLength = 1;
	int low, high;
	for (int i = 0; i != n; ++i) 
	{
		low = i - 1;
		high = i + 1;
		while (high < n && str[high] == str[i]) // increment 'high'
			high++;
		while (low >= 0 && str[low] == str[i]) // decrement 'low'
			low--;
		while (low >= 0 && high < n && str[low] == str[high]) {
			low--; // decrement low
			high++; // increment high
		}
		int length = high - low - 1;
		if (maxLength < length) 
            maxLength = length;
	}
	return maxLength;
}


int main()
{
	char str[] = "alsfjjfllk";
	printf("Length is: %d\n", lps(str));
	return 0;
}
