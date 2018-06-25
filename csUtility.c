#include "csUtility.h"

int randInt(int low, int high, bool inclusive)
{
    return low + (rand() % (high + inclusive));
}

/** \brief converts any int to a string.
 *
 * \param value - what value you want
 * \param result - valid pointer to get your string
 * \return pointer to your inputted string (for convenience)
 */
char* intToString(int value, char* result)
{
    if (value == 0)
        return "0";
    bool negFlag = false;
    if (value < 0)
    {
        negFlag = true;
        value *= -1;
        //printf("new value = %d\n", value);
    }
	const int digit = digits(value);
	//printf("digit = %d\n", digit);
	result = calloc(digit + 1 + negFlag, sizeof(char));
	result[digit + negFlag] = '\0';
	int usedVal = 0;
	for (int i = digit; i > 0; i--)
	{
		int x = (value - usedVal) / (int) pow(i - 1, 10);
		result[digit - i] = (char) x + '0';
		//printf("result[%d] = (%d) / %d = %d = character %c\n", digit - i, value - usedVal, pwrOf10(i - 1), x, result[digit - i]);
		usedVal = usedVal + (result[digit - i] - '0') * (int) pow(i - 1, 10);
		//printf("usedVal = itself + %d * %d = %d\n", (int) result[digit - i] - '0', pwrOf10(i - 1), usedVal);
	}
	if (negFlag)
    {
        char negative[digit + 1];
        negative[0] = '-';
        strcat(negative, result);
        strcpy(result, negative);
    }
	//printf("%s\n",result);
	return result;
}

/** \brief Gets the amount of digits in num.
 *
 * \param num - your number
 * \return the number of digits in num.
 */
int digits(int num)
{
    if (num < 0)
        num *= -1;
	return 1 + log10(num);
}

/** \brief free() with automatic NULL setting.
 * Use: x = freeThisMem(x);
 * \param x - memory address
 * \return NULL
 *
 */
void* freeThisMem(void* x)
{
	free(x);
	return NULL;
}

char* removeNewline(char* stuff, char replacement, int maxLength)
{
    for(int i = maxLength - 1; i >= 0; i--)
    {
        if (i < 0)
            return stuff;
        if (stuff[i] == '\n')
        {
            stuff[i] = replacement;
        }
    }
    return stuff;
}
