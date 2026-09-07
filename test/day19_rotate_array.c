#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static void reverse_range(int *first, int *last)
{
    /* TODO：翻转区间[first, last) */
    if(first==NULL||last==NULL)
    {
        return;
    }
   while(first<last)
    {
        --last;
        if(first>=last)
        {
            break;
        }

        int temp=*first;
        *first=*last;
        *last=temp;

        ++first;
    }
}

static void rotate_right(int *array, size_t length, size_t k)
{
    /* TODO：将数组向右旋转k位 */
    if(array==NULL||length<2)
    {
        return;
    }

    k%=length;

    if(k==0)
    {
        return;
    }

    reverse_range(array,array+length);
    reverse_range(array,array+k);
    reverse_range(array+k,array+length);
}

static int arrays_equal(const int *a, const int *b, size_t length)
{
    for (size_t i = 0U; i < length; ++i)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }

    return 1;
}

int main(void)
{
    int a[] = {1, 2, 3, 4, 5};
    const int expected_a[] = {4, 5, 1, 2, 3};

    rotate_right(a, 5U, 2U);
    assert(arrays_equal(a, expected_a, 5U));

    int b[] = {1, 2, 3, 4, 5};
    const int expected_b[] = {4, 5, 1, 2, 3};

    rotate_right(b, 5U, 7U);
    assert(arrays_equal(b, expected_b, 5U));

    int c[] = {8};
    rotate_right(c, 1U, 100U);
    assert(c[0] == 8);

    rotate_right(NULL, 0U, 3U);

    puts("Day19 rotate array: all checks passed.");
    return 0;
}