#include "domain/CommandParser.hpp"

#include <gtest/gtest.h>
#include <string_view>

namespace p3d::test
{
    namespace
    {
        std::optional<domain::Command> feedString(domain::CommandParser& parser,
                                                  std::string_view       str)
        {
            std::optional<domain::Command> last{};
            for (const char c : str)
            {
                const auto cmd = parser.feed(c);
                if (cmd.has_value())
                {
                    last = cmd;
                }
            }
            return last;
        }
    }

    TEST(CommandParserTest, ImmediateSingleCharZeroShortcut)
    {
        domain::CommandParser parser{};

        const auto cmdLower = parser.feed('z');
        ASSERT_TRUE(cmdLower.has_value());
        EXPECT_EQ(cmdLower->type, domain::CommandType::CalibrateCenter);

        const auto cmdUpper = parser.feed('Z');
        ASSERT_TRUE(cmdUpper.has_value());
        EXPECT_EQ(cmdUpper->type, domain::CommandType::CalibrateCenter);
    }

    TEST(CommandParserTest, ParsesAllLineBasedCommands)
    {
        domain::CommandParser parser{};

        auto cmd = feedString(parser, "CAL_CENTER\r\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::CalibrateCenter);

        cmd = feedString(parser, "CAL_MINMAX_START\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::StartMinMaxCapture);

        cmd = feedString(parser, "CAL_MINMAX_END\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::EndMinMaxCapture);

        cmd = feedString(parser, "SAVE_CAL\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::SaveCalibration);

        cmd = feedString(parser, "LOAD_CAL\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::LoadCalibration);

        cmd = feedString(parser, "RESET_CAL\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::ResetDefaults);

        cmd = feedString(parser, "GET_CAL\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::QueryCalibration);

        cmd = feedString(parser, "BOOTSEL\n");
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, domain::CommandType::EnterBootloader);
    }

    TEST(CommandParserTest, ParsesParameterizedSetDeadzoneAndSetInvert)
    {
        domain::CommandParser parser{};

        auto dzCmd = feedString(parser, "SET_DZ 2 25\n");
        ASSERT_TRUE(dzCmd.has_value());
        EXPECT_EQ(dzCmd->type, domain::CommandType::SetDeadzone);
        EXPECT_EQ(dzCmd->axisIndex, 2U);
        EXPECT_EQ(dzCmd->uintParam, 25U);

        auto invCmd = feedString(parser, "SET_INV 1 1\n");
        ASSERT_TRUE(invCmd.has_value());
        EXPECT_EQ(invCmd->type, domain::CommandType::SetInvert);
        EXPECT_EQ(invCmd->axisIndex, 1U);
        EXPECT_TRUE(invCmd->boolParam);

        auto invOffCmd = feedString(parser, "SET_INV 0 0\n");
        ASSERT_TRUE(invOffCmd.has_value());
        EXPECT_EQ(invOffCmd->type, domain::CommandType::SetInvert);
        EXPECT_EQ(invOffCmd->axisIndex, 0U);
        EXPECT_FALSE(invOffCmd->boolParam);
    }

    TEST(CommandParserTest, RejectsInvalidAxisOrMalformedParameters)
    {
        EXPECT_FALSE(domain::CommandParser::parseLine("SET_DZ 3 20").has_value());
        EXPECT_FALSE(domain::CommandParser::parseLine("SET_DZ 0 3000").has_value());
        EXPECT_FALSE(domain::CommandParser::parseLine("SET_DZ 0").has_value());
        EXPECT_FALSE(domain::CommandParser::parseLine("SET_INV 1 2").has_value());
        EXPECT_FALSE(domain::CommandParser::parseLine("UNKNOWN_CMD").has_value());
    }

    TEST(CommandParserTest, RecoversCleanlyAfterBufferOverflow)
    {
        domain::CommandParser parser{};

        for (std::size_t i = 0; i < 80; ++i)
        {
            EXPECT_FALSE(parser.feed('A').has_value());
        }
        EXPECT_FALSE(parser.feed('\n').has_value());

        const auto next = feedString(parser, "BOOTSEL\n");
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(next->type, domain::CommandType::EnterBootloader);
    }
}
