//
// Created by marcel on 12/23/17.
//

#include <catch.hpp>
#include <checked_cmd.h>
#include <cstdint>
#include <cstring>

namespace ArgumentsTest::detail {
        auto CopyToArgs(std::initializer_list<std::string> const& list){
            std::vector<std::unique_ptr<char[]>> result;
            for (auto const &item: list){
                result.push_back(std::make_unique<char[]>(item.size()+1));
                std::strncpy(result.back().get(), item.c_str(), item.size()+1);
            }
            return result;
        }
        auto IntoPtrs(std::vector<std::unique_ptr<char[]>> & args){
            auto result = new char*[args.size()];
            std::transform(std::begin(args), std::end(args), result, [](auto & uptr){ return uptr.get();});
            return result;
        }
}
namespace ArgumentsTest {

    #define TYPE_SAFE(Type, Name) using Name = CheckedCmdTypesafe::Typesafe<Type, struct TypeTag##Name>;

    using namespace CheckedCmd;

    TYPE_SAFE(uint16_t,    ExcelRow)
    TYPE_SAFE(bool,        HasHeadLine)
    TYPE_SAFE(std::string, InputFile)
    TYPE_SAFE(std::string, SecInputFile)
    TYPE_SAFE(std::string, OutputFile)
    TYPE_SAFE(std::string, TableName)
    TYPE_SAFE(char,        CsvSep)

    using CmdHasHeadLine     = CheckedCmd::Flag<HasHeadLine>;
    using CmdOutputFile      = CheckedCmd::Param<std::optional<OutputFile>>;
    using CmdInputFile       = CheckedCmd::Arg<InputFile>;
    using CmdSecondInputFile = CheckedCmd::Arg<SecInputFile>;
    using CmdOptArg          = CheckedCmd::Arg<std::optional<std::string>>;
    using CmdExcelRowLimit   = CheckedCmd::Param<std::optional<ExcelRow>>;
    using CmdTableName       = CheckedCmd::Param<TableName>;
    using OptCmdCsvSep       = CheckedCmd::Param<std::optional<CsvSep>>;
    using CmdCsvSep          = CheckedCmd::Param<CsvSep>;

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

    TEST_CASE ("RequriedParamater") {
        auto const config = std::make_tuple(CmdTableName(Hint(""), ShortName("-T"),
                                                         LongName("--TableName"),
                                                         Description("lol"),
                                                         TableNameValidator
                                            )
        );
        SECTION ("missing leads to failure") {
            auto args = detail::CopyToArgs({"prgname", "-l 2", "-H", "-h", "file.csv", "file1.csv", "string"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(!success.has_value());
        }

        SECTION ("successfully parsed but failing check leads to failure") {
            auto args = detail::CopyToArgs({"prgname", "-T files"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(!success.has_value());
        }

        SECTION ("successfully parsed and passing check leads to success") {
            auto args = detail::CopyToArgs({"prgname", "-T file"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
        }
    }
    TEST_CASE ("OptionalParameter") {
        auto const config = std::make_tuple(CmdOutputFile(Hint(""), ShortName("-O"),
                                                          LongName("--OutFile"),
                                                          Description("lol"),
                                                          OutputFileValidator
                                            )
        );
        SECTION ("successfully parsed and passing check leads to success") {
            auto args = detail::CopyToArgs({"prgname", "-O file"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
        }

        SECTION ("successfully parsed and failing check leads to failure") {
            auto args = detail::CopyToArgs({"prgname", "-O outfile"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(!success.has_value());
        }

        SECTION ("not parsed leads to success") {
            auto args = detail::CopyToArgs({"prgname"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
        }
    }
    TEST_CASE("std::optional<Typesafe<char,typetag>>"){
        auto const config = std::make_tuple(OptCmdCsvSep(Hint(""), ShortName("-S"),
                                                          LongName("--Separator"),
                                                          Description("lol"),
                                                          NoChecks
                                                     )
                                           );

        SECTION ("without quotes") {
            auto args = detail::CopyToArgs({"prgname","-S ,"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<OptCmdCsvSep>(success.value()).value_or(CsvSep('@')) == CsvSep(','));
        }
        SECTION ("with single quotes") {
            auto args = detail::CopyToArgs({"prgname","-S ','"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<OptCmdCsvSep>(success.value()).value_or(CsvSep('@')) == CsvSep(','));
        }
        SECTION ("with double quotes") {
            auto args = detail::CopyToArgs({"prgname",R"(-S ",")"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<OptCmdCsvSep>(success.value()).value_or(CsvSep('@')) == CsvSep(','));
        }
    }

    TEST_CASE("Typesafe<char,typetag>"){
        auto const config = std::make_tuple(CmdCsvSep(Hint(""), ShortName("-S"),
                                                      LongName("--Separator"),
                                                      Description("lol"),
                                                      NoChecks
                                            )
        );

        SECTION ("without quotes") {
            auto args = detail::CopyToArgs({"prgname","-S ,"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<CmdCsvSep>(success.value()).value() == CsvSep(','));
        }
        SECTION ("with single quotes") {
            auto args = detail::CopyToArgs({"prgname","-S ','"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<CmdCsvSep>(success.value()).value() == CsvSep(','));
        }
        SECTION ("with double quotes") {
            auto args = detail::CopyToArgs({"prgname",R"(-S ",")"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(success.has_value());
            REQUIRE(std::get<CmdCsvSep>(success.value()).value() == CsvSep(','));
        }
        SECTION ("missing fails") {
            auto args = detail::CopyToArgs({"prgname"});
            auto success = ParseCmdArgsTuple(args.size(), detail::IntoPtrs(args), config );
            CHECK(!success.has_value());
        }

    }

    TEST_CASE ("full example") {
        auto args = detail::CopyToArgs({"prgname", "-l 2",  "-H", "-h", "file.csv", "file1.csv", "string"});
        auto argv = detail::IntoPtrs(args);
        auto argc = args.size();
        auto const success = ParseCmd(argc, argv
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
            REQUIRE ( InputFile("file.csv")     == std::get<CmdInputFile>(parsed_args).value());
            REQUIRE ( SecInputFile("file1.csv") == std::get<CmdSecondInputFile>(parsed_args).value());
            REQUIRE ( HasHeadLine(true)         == std::get<CmdHasHeadLine>(parsed_args).value());
            REQUIRE ( OutputFile("lol")         == std::get<CmdOutputFile>(parsed_args).value_or(OutputFile("lol")));
            REQUIRE ( true                      == std::get<CheckedCmd::Help>(parsed_args).value());
            REQUIRE ( true                      == std::get<CmdOptArg>(parsed_args).value().has_value());
        }
    }
}