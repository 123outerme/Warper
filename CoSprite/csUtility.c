#include "csUtility.h"

int randInt(int low, int high, bool inclusive)
{
    return low + (rand() % (high + inclusive));
}

/** \brief Gets the amount of digits in num.
 *
 * \param num - your number
 * \return the number of digits in num.
 */
int digits(int num)
{
    if (num < 0)
    {
        num *= -1;
    }

	return 1 + log10(num);
}

/** \brief free() with automatic NULL setting.
 * Use: x = freeThisMem(x);
 * \param x - memory address
 * \return NULL
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


double getDistance(double x1, double y1, double x2, double y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
