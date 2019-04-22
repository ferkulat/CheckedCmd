//
// Created by marcel on 12/23/17.
//

#include <doctest.h>
#include <checked_cmd.h>
#include <cstdint>

namespace ArgumentsTest {
    #define TYPE_SAFE(Type, Name) using Name = CheckedCmdTypesafe::Typesafe<Type, struct TypeTag##Name>;

    using namespace CheckedCmd;

    TYPE_SAFE(uint16_t,    ExcelRow)
    TYPE_SAFE(bool,        HasHeadLine)
    TYPE_SAFE(std::string, InputFile)
    TYPE_SAFE(std::string, SecInputFile)
    TYPE_SAFE(std::string, OutputFile)
    TYPE_SAFE(std::string, TableName)

    using CmdHasHeadLine     = CheckedCmd::Flag<HasHeadLine>;
    using CmdOutputFile      = CheckedCmd::Param<std::optional<OutputFile>>;
    using CmdInputFile       = CheckedCmd::Arg<InputFile>;
    using CmdSecondInputFile = CheckedCmd::Arg<SecInputFile>;
    using CmdOptArg          = CheckedCmd::Arg<std::optional<std::string>>;
    using CmdExcelRowLimit   = CheckedCmd::Param<std::optional<ExcelRow>>;
    using CmdTableName       = CheckedCmd::Param<TableName>;

    bool OutputFileValidator(OutputFile const &outputFile) {
        return outputFile.Get().size() < 6;
    }

    bool InputFileValidator(InputFile const &inputFile) {
        return inputFile.Get().size() < 100;
    }

    bool RowLimitValidator(ExcelRow const &excelRow) {
        return (excelRow.Get() > 0);
    }

    bool TableNameValidator(TableName const &tableName) {
        return (tableName.Get().size() < 5);
    }

    auto const NoChecks = [](auto const &) { return true; };

    TEST_SUITE ("RequriedParamater") {
        auto const config = std::make_tuple(CmdTableName(Hint(""), ShortName("-T"),
                                                         LongName("--TableName"),
                                                         Description("lol"),
                                                         TableNameValidator
                                            )
        );
        TEST_CASE ("missing leads to failure") {
            auto success = ParseCmdArgsTuple({"prgname", "-l 2", "-H", "-h", "file.csv", "file1.csv", "string"},
                                             config
            );
            CHECK(!success.has_value());
        }

        TEST_CASE ("successfully parsed but failing check leads to failure") {
            auto success = ParseCmdArgsTuple({"prgname", "-T files"}, config);
            CHECK(!success.has_value());
        }

        TEST_CASE ("successfully parsed and passing check leads to success") {
            auto success = ParseCmdArgsTuple({"prgname", "-T file"}, config);
            CHECK(success.has_value());
        }
    }
    TEST_SUITE ("OptionalParameter") {
        auto const config = std::make_tuple(CmdOutputFile(Hint(""), ShortName("-O"),
                                                          LongName("--OutFile"),
                                                          Description("lol"),
                                                          OutputFileValidator
                                            )
        );
        TEST_CASE ("successfully parsed and passing check leads to success") {
            auto success = ParseCmdArgsTuple({"prgname", "-O file"}, config);
            CHECK(success.has_value());
        }

        TEST_CASE ("successfully parsed and failing check leads to failure") {
            auto success = ParseCmdArgsTuple({"prgname", "-O outfile"}, config);
            CHECK(!success.has_value());
        }

        TEST_CASE ("not parsed leads to success") {
            auto success = ParseCmdArgsTuple({"prgname"}, config);
            CHECK(success.has_value());
        }
    }
    TEST_CASE ("full example") {

        auto const success = ParseCmd({"prgname", "-l 2",  "-H", "-h", "file.csv", "file1.csv", "string"}
                                ,CmdHasHeadLine(ShortName("-H")
                                                ,LongName("--HasHeadLine")
                                                ,Description("lol")
                                               )
                                ,CmdOutputFile(Hint("filename")
                                               ,ShortName("-o")
                                               ,LongName("--OutPutFile")
                                               ,Description("")
                                               ,OutputFileValidator
                                              )
                                ,CmdInputFile(Hint("inputfile")
                                              ,Description("lol")
                                              ,InputFileValidator
                                             )
                                ,CmdSecondInputFile(Hint("outputfile")
                                                    ,Description("lol")
                                                    ,NoChecks
                                                   )
                                ,CmdOptArg(Hint("CmdOptArg")
                                           ,Description("lol")
                                           ,NoChecks
                                          )
                                ,CmdExcelRowLimit(Hint("1..65535")
                                                 ,ShortName("-l")
                                                 ,LongName("--LineLimit")
                                                 ,Description(R"(If option "-H" is set, the minimum allowed value is 2.)")
                                                 ,RowLimitValidator
                                                )
                                ,CheckedCmd::Help()
        );

        CHECK(success);

        if (success) {
            auto const parsed_args = success.value();
            CHECK_EQ ( InputFile("file.csv"),     std::get<CmdInputFile>(parsed_args).value());
            CHECK_EQ ( SecInputFile("file1.csv"), std::get<CmdSecondInputFile>(parsed_args).value());
            CHECK_EQ ( HasHeadLine(true),         std::get<CmdHasHeadLine>(parsed_args).value());
            CHECK_EQ ( OutputFile("lol"),         std::get<CmdOutputFile>(parsed_args).value_or(OutputFile("lol")));
            CHECK_EQ ( true,                      std::get<CheckedCmd::Help>(parsed_args).value());
            CHECK_EQ ( true,                      std::get<CmdOptArg>(parsed_args).value().has_value());
        }
    }
}