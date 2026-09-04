#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define BRACKET_STACK_CAPACITY 128U

static bool IsMatchingPair(char left, char right)
{
    /*
     * TODO 1：
     * ()、[]、{}分别返回true，
     * 其他组合返回false。
     */

    return (((left == '(') && (right == ')')) ||
            ((left == '[') && (right == ']')) ||
            ((left == '{') && (right == '}')));
}



static bool IsValidBrackets(const char *text)
{
    char stack[BRACKET_STACK_CAPACITY];
    size_t top = 0U;

    if (text == NULL)
    {
        return false;
    }

    for (size_t index = 0U;
         text[index] != '\0';
         index++)
    {
        char current = text[index];

        if ((current == '(') ||
            (current == '[') ||
            (current == '{'))
        {
            /* TODO 2.1：先检查栈是否已满，再将左括号入栈 */
            if(top>=BRACKET_STACK_CAPACITY)
            {
                return false;
            }
            stack[top]=current;
            top++;
            
        }
        else if ((current == ')') ||
                 (current == ']') ||
                 (current == '}'))
        {
            /*
             * TODO 2.2：
             * 1. 检查栈是否为空
             * 2. 弹出栈顶元素
             * 3. 调用IsMatchingPair()检查
             */
            if(top==0U)
            {
                return false;
            }
            
              char left=stack[--top];
            if (!IsMatchingPair(left, current))
            {
                return false;
            }
         
            
        }
        else
        {
            /* 不属于六种括号的字符视为非法输入 */
            return false;
        }
    }

    return top == 0U;
}

static void TestValidCases(void)
{
    assert(IsValidBrackets(""));
    assert(IsValidBrackets("()"));
    assert(IsValidBrackets("()[]{}"));
    assert(IsValidBrackets("{[()]}"));
    assert(IsValidBrackets("(([]){})"));
}

static void TestInvalidCases(void)
{
    assert(!IsValidBrackets("(]"));
    assert(!IsValidBrackets("([)]"));
    assert(!IsValidBrackets("((("));
    assert(!IsValidBrackets("]"));
    assert(!IsValidBrackets("abc"));
    assert(!IsValidBrackets(NULL));
}

static void TestStackOverflow(void)
{
    char too_deep[BRACKET_STACK_CAPACITY + 2U];

    for (size_t index = 0U;
         index < BRACKET_STACK_CAPACITY + 1U;
         index++)
    {
        too_deep[index] = '(';
    }

    too_deep[BRACKET_STACK_CAPACITY + 1U] = '\0';

    assert(!IsValidBrackets(too_deep));
}

int main(void)
{
    TestValidCases();
    TestInvalidCases();
    TestStackOverflow();

    printf("Day16 valid brackets tests passed\n");
    return 0;
}