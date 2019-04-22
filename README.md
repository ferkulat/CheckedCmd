[![Build Status](https://travis-ci.org/ferkulat/CheckedCmd.svg?branch=master)](https://travis-ci.org/ferkulat/CheckedCmd)

# CheckedCmd

This is a command line parser library which is typed an checked. It makes use of [Clara](https://github.com/catchorg/Clara) to do all the hard work behind the scenes.

it does the following: 
* creates a tuple of the passed types or takes a tuple by value,
* assigns the parsed values to each of the tuple members
* calls the checks for each of them
* and returns ```std::optional<std::tuple<...>>```

The caller does not need to use mutable variables.

The parser needs distinct types as input.
```c++
using CmdHasHeadLine   = CheckedCmd::Flag<HasHeadLine>;                //optional  
using CmdOutputFile    = CheckedCmd::Param<std::optional<OutputFile>>; //optional
using CmdExcelRowLimit = CheckedCmd::Param<std::optional<ExcelRow>>;
using CmdTableName     = CheckedCmd::Param<TableName>;                //required
using CmdInputFile     = CheckedCmd::Arg<InputFile>;                  //required
using CmdOptArg        = CheckedCmd::Arg<std::optional<std::string>>; //optional
```
These types need to be default constructable and provide an overload for 
```c++
std::istream& operator>>(std::istream&, type&);
```  
We need to provide callables for range checks for each type:
```c++
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
```

Now the input gets parsed without introducing state.
``` ParseCmd``` returns a ```std::optional<std::tuple<...>>```:
```c++
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

```
The parsed values can be accessed by:
```c++
if (success.has_value()) {
    auto parsed_args = success.value();
    CHECK_EQ ( InputFile("file.csv"),     std::get<CmdInputFile>(parsed_args).value());
    CHECK_EQ ( SecInputFile("file1.csv"), std::get<CmdSecondInputFile>(parsed_args).value());
    CHECK_EQ ( HasHeadLine(true),         std::get<CmdHasHeadLine>(parsed_args).value());
    CHECK_EQ ( OutputFile("lol"),         std::get<CmdOutputFile>(parsed_args).value_or(OutputFile("lol")));
    CHECK_EQ ( true,                      std::get<CheckedCmd::Help>(parsed_args).value());
    CHECK_EQ ( true,                      std::get<CmdOptArg>(parsed_args).value().has_value());
}
```
If parsing or one of the checks fail, the returned value is ```std::nullopt```.
 
