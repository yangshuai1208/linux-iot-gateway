#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRAME_MAGIC_1       0xAAU
#define FRAME_MAGIC_2       0x55U
#define MAX_PAYLOAD_SIZE    16U

typedef enum
{
    PARSER_WAIT_MAGIC_1 = 0,
    PARSER_WAIT_MAGIC_2,
    PARSER_READ_TYPE,
    PARSER_READ_LENGTH,
    PARSER_READ_PAYLOAD,
    PARSER_READ_CHECKSUM
} ParserState;

typedef enum
{
    PARSER_WAITING = 0,
    PARSER_FRAME_READY,
    PARSER_BAD_LENGTH,
    PARSER_BAD_CHECKSUM,
    PARSER_INVALID_ARGUMENT
} ParserResult;

typedef struct
{
    uint8_t type;
    uint8_t length;
    uint8_t payload[MAX_PAYLOAD_SIZE];
} ProtocolFrame;

typedef struct
{
    ParserState state;
    ProtocolFrame working_frame;
    size_t payload_index;
    uint8_t calculated_checksum;
} FrameParser;

static void parser_init(FrameParser *parser)
{
    if (parser == NULL)
    {
        return;
    }

    memset(parser, 0, sizeof(*parser));
    parser->state = PARSER_WAIT_MAGIC_1;
}

static ParserResult parser_feed_byte(
    FrameParser *parser,
    uint8_t byte,
    ProtocolFrame *output)
{
    /*
     * TODO 1：检查parser和output。
     */
    if(parser==NULL||output==NULL)
    {
      return PARSER_INVALID_ARGUMENT;
    }

    /*
     * TODO 2：使用switch处理六个状态。
     *
     * WAIT_MAGIC_1：
     *   收到0xAA后进入WAIT_MAGIC_2。
     *
     * WAIT_MAGIC_2：
     *   收到0x55后读取TYPE。
     *   再次收到0xAA时继续等待0x55。
     *   其他字节回到WAIT_MAGIC_1。
     *
     * READ_TYPE：
     *   保存type，初始化checksum。
     *
     * READ_LENGTH：
     *   检查长度是否超过MAX_PAYLOAD_SIZE。
     *   长度为0直接进入CHECKSUM。
     *
     * READ_PAYLOAD：
     *   保存Payload并更新异或校验。
     *
     * READ_CHECKSUM：
     *   校验成功才修改output。
     *   无论成功失败都要恢复初始状态。
     */
    switch(parser->state)
    {
        case PARSER_WAIT_MAGIC_1:
    {    if(byte==FRAME_MAGIC_1 )
        {
            parser->state=PARSER_WAIT_MAGIC_2;
        }
        return PARSER_WAITING;
    }
    
    
        case PARSER_WAIT_MAGIC_2:
        {
        if(byte==FRAME_MAGIC_2)
        {
            parser->state=PARSER_READ_TYPE;
        }
        else if(byte==FRAME_MAGIC_1)
        {
            parser->state=PARSER_WAIT_MAGIC_2;
        }
        else
        {
            parser->state=PARSER_WAIT_MAGIC_1;
        }
        return PARSER_WAITING;
        }
        case PARSER_READ_TYPE:
        {
            parser->working_frame.type=byte;
            parser->calculated_checksum=byte;
            parser->payload_index=0U;
            parser->state=PARSER_READ_LENGTH;
            
            return PARSER_WAITING;
        }
        
        case PARSER_READ_LENGTH:
        {
            if(byte>MAX_PAYLOAD_SIZE)
            {
                parser_init(parser);
                return PARSER_BAD_LENGTH;
            }
            parser->working_frame.length=byte;
            parser->calculated_checksum=(uint8_t)(parser->calculated_checksum^byte);
            parser->payload_index=0U;

            if(byte==0)
            {
                parser->state=PARSER_READ_CHECKSUM;
            }
            else
            {
                parser->state=PARSER_READ_PAYLOAD;
            }
            return PARSER_WAITING;
        }
        
        case PARSER_READ_PAYLOAD:
        {

            parser->working_frame.payload[parser->payload_index]=byte;
            parser->calculated_checksum=(uint8_t)(parser->calculated_checksum^byte);
            ++parser->payload_index;
            if(parser->payload_index==parser->working_frame.length)
            {
                parser->state=PARSER_READ_CHECKSUM;
            }
            return PARSER_WAITING;
        }
        case PARSER_READ_CHECKSUM:
        {
            if(byte!=parser->calculated_checksum)
            {
                parser_init(parser);
                return PARSER_BAD_CHECKSUM;
            }
            *output=parser->working_frame;

            parser_init(parser);
            return PARSER_FRAME_READY;
        }
        default:
        {
            parser_init(parser);
            return PARSER_INVALID_ARGUMENT;
        }

    }   
}

static void test_valid_frame(void)
{
    FrameParser parser;
    ProtocolFrame output = {0};

    const uint8_t data[] =
    {
        0xAAU,
        0x55U,
        0x01U,
        0x03U,
        0x41U,
        0x42U,
        0x43U,
        0x42U
    };

    parser_init(&parser);

    for (size_t index = 0U;
         index < sizeof(data) - 1U;
         ++index)
    {
        assert(parser_feed_byte(
            &parser,
            data[index],
            &output
        ) == PARSER_WAITING);
    }

    assert(parser_feed_byte(
        &parser,
        data[sizeof(data) - 1U],
        &output
    ) == PARSER_FRAME_READY);

    assert(output.type == 0x01U);
    assert(output.length == 3U);
    assert(output.payload[0] == 0x41U);
    assert(output.payload[1] == 0x42U);
    assert(output.payload[2] == 0x43U);
}
static void test_sticky_frames(void)
{
    FrameParser parser;
    ProtocolFrame output = {0};

    const uint8_t stream[] =
    {
        /* 第一帧：type=1，payload={0x10} */
        0xAAU, 0x55U,
        0x01U, 0x01U,
        0x10U,
        0x10U,

        /* 第二帧：type=2，payload={0x20, 0x21} */
        0xAAU, 0x55U,
        0x02U, 0x02U,
        0x20U, 0x21U,
        0x01U
    };

    size_t ready_count = 0U;

    parser_init(&parser);

    for (size_t index = 0U;
         index < sizeof(stream);
         ++index)
    {
        ParserResult result =
            parser_feed_byte(
                &parser,
                stream[index],
                &output
            );

        if (result == PARSER_FRAME_READY)
        {
            if (ready_count == 0U)
            {
                assert(output.type == 0x01U);
                assert(output.length == 1U);
                assert(output.payload[0] == 0x10U);
            }
            else
            {
                assert(ready_count == 1U);
                assert(output.type == 0x02U);
                assert(output.length == 2U);
                assert(output.payload[0] == 0x20U);
                assert(output.payload[1] == 0x21U);
            }

            ++ready_count;
        }
        else
        {
            assert(result == PARSER_WAITING);
        }
    }

    assert(ready_count == 2U);
}
static void test_bad_length(void)
{
    FrameParser parser;

    ProtocolFrame output =
    {
        0xEEU,
        0xEEU,
        {0xEEU}
    };

    const uint8_t data[] =
    {
        0xAAU,
        0x55U,
        0x01U,
        MAX_PAYLOAD_SIZE + 1U
    };

    parser_init(&parser);

    for (size_t index = 0U;
         index < sizeof(data) - 1U;
         ++index)
    {
        assert(parser_feed_byte(
            &parser,
            data[index],
            &output
        ) == PARSER_WAITING);
    }

    assert(parser_feed_byte(
        &parser,
        data[sizeof(data) - 1U],
        &output
    ) == PARSER_BAD_LENGTH);

    /* 失败不能修改output。 */
    assert(output.type == 0xEEU);

    /* 错误后必须重新等待帧头。 */
    assert(parser.state == PARSER_WAIT_MAGIC_1);
}
static void test_bad_checksum_and_recovery(void)
{
    FrameParser parser;

    ProtocolFrame output =
    {
        0xEEU,
        0xEEU,
        {0xEEU}
    };

    const uint8_t bad_frame[] =
    {
        0xAAU, 0x55U,
        0x01U, 0x01U,
        0x10U,

        /* 正确校验应为0x10。 */
        0x00U
    };

    const uint8_t good_frame[] =
    {
        0xAAU, 0x55U,
        0x01U, 0x01U,
        0x10U,
        0x10U
    };

    parser_init(&parser);

    for (size_t index = 0U;
         index < sizeof(bad_frame) - 1U;
         ++index)
    {
        assert(parser_feed_byte(
            &parser,
            bad_frame[index],
            &output
        ) == PARSER_WAITING);
    }

    assert(parser_feed_byte(
        &parser,
        bad_frame[sizeof(bad_frame) - 1U],
        &output
    ) == PARSER_BAD_CHECKSUM);

    assert(output.type == 0xEEU);
    assert(parser.state == PARSER_WAIT_MAGIC_1);

    /* 错误帧之后继续输入正确帧。 */
    for (size_t index = 0U;
         index < sizeof(good_frame) - 1U;
         ++index)
    {
        assert(parser_feed_byte(
            &parser,
            good_frame[index],
            &output
        ) == PARSER_WAITING);
    }

    assert(parser_feed_byte(
        &parser,
        good_frame[sizeof(good_frame) - 1U],
        &output
    ) == PARSER_FRAME_READY);

    assert(output.type == 0x01U);
    assert(output.length == 1U);
    assert(output.payload[0] == 0x10U);
}
static void test_noise_and_repeated_magic(void)
{
    FrameParser parser;
    ProtocolFrame output = {0};

    const uint8_t data[] =
    {
        0x99U,       /* 噪声 */
        0xAAU,
        0xAAU,       /* 新帧头开始 */
        0x55U,
        0x03U,       /* type */
        0x00U,       /* 零长度Payload */
        0x03U        /* checksum = 03 ^ 00 */
    };

    parser_init(&parser);

    for (size_t index = 0U;
         index < sizeof(data) - 1U;
         ++index)
    {
        assert(parser_feed_byte(
            &parser,
            data[index],
            &output
        ) == PARSER_WAITING);
    }

    assert(parser_feed_byte(
        &parser,
        data[sizeof(data) - 1U],
        &output
    ) == PARSER_FRAME_READY);

    assert(output.type == 0x03U);
    assert(output.length == 0U);
}
int main(void)
{
    test_valid_frame();
    test_sticky_frames();
    test_bad_length();
    test_bad_checksum_and_recovery();
    test_noise_and_repeated_magic();

    printf("length-prefixed framer tests passed\n");
    return 0;
}