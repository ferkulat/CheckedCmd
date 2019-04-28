//
// Created by marcel on 12/23/17.
//

#ifndef CHECKED_CMD_H
#define CHECKED_CMD_H
#include <utility>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <optional>
#include <vector>
#include <clara.hpp>
#include <functional>
#include <regex>

namespace CheckedCmd{
    namespace CheckedCmdTypesafe{
        template< typename T, typename TypeTag>
        class Typesafe;
        template< typename T, typename TypeTag>
        std::istream& operator>>(std::istream& is, Typesafe<T, TypeTag> &target);
        template< typename T, typename TypeTag>
        std::ostream& operator<<(std::ostream& os, Typesafe<T, TypeTag> const& target);
        template< typename T, typename TypeTag>
        class Typesafe{
            T value;
        public:
            Typesafe()=default;
            constexpr explicit Typesafe(T t_):value(std::move(t_)){}
            constexpr T const& Get()const{return value;}
            constexpr bool operator==(Typesafe const& other)const{
                return value == other.value;
            }
        private:
            friend std::istream& operator>> <T, TypeTag>(std::istream& is, Typesafe<T, TypeTag> &target);
            friend std::ostream& operator<< <T, TypeTag>(std::ostream& os, Typesafe<T, TypeTag> const& target);

        };
        template< typename T, typename TypeTag>
        std::istream& operator>>(std::istream& is, Typesafe<T, TypeTag> &target){
            is >> target.value;
            return is;
        }
        template< typename T, typename TypeTag>
        std::ostream& operator<<(std::ostream& os, Typesafe<T, TypeTag> const& target){
            os << target.value;
            return os;
        }
    }

    namespace Helpers{
        //http://aherrmann.github.io/programming/2016/02/28/unpacking-tuples-in-cpp14/
        namespace detail {
            template<class F, std::size_t... Is>
            constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
                return f(std::integral_constant<std::size_t, Is>{}...);
            }

            template<std::size_t N, class F>
            constexpr auto index_apply(F f) {
                return index_apply_impl(f, std::make_index_sequence<N>{});
            }
        }
        template <class Tuple>
        constexpr auto reverse(Tuple t) {
            return detail::index_apply<std::tuple_size<Tuple>{}>(
                [&](auto... Is) {
                    return make_tuple(
                        std::get<std::tuple_size<Tuple>{} - 1 - Is>(t)...);
                });
        }
    }

    using Description = CheckedCmdTypesafe::Typesafe<std::string, struct TypeTagDescription>;
    using ShortName   = CheckedCmdTypesafe::Typesafe<std::string, struct TypeTagShortName>;
    using LongName    = CheckedCmdTypesafe::Typesafe<std::string, struct TypeTagLongName>;
    using Hint        = CheckedCmdTypesafe::Typesafe<std::string, struct TypeTagHint>;
    using HelpFlag    = CheckedCmdTypesafe::Typesafe<bool,        struct TypeTagHelpFlag>;

    template<typename Type>
    struct ClaraArgBinder;

    template<typename TypeTag>
    struct ClaraFlagBinder{
    };

    template<typename Type>
    struct ClaraParamBinder;

    template<typename TypeTag>
    class Flag{};
    template<typename TypeTag>
    struct ClaraFlagBinder<CheckedCmdTypesafe::Typesafe<bool, TypeTag>>{
        static clara::Parser bind(clara::Parser const& parser, Flag<CheckedCmdTypesafe::Typesafe<bool,TypeTag>>& t){
            return parser|clara::Opt(t.boolval)
            [t.GetShortName().Get()][t.GetName().Get()]
                    (t.GetDescription().Get());
        }

    };


    template<typename TypeTag>
    class Flag<CheckedCmdTypesafe::Typesafe<bool, TypeTag>>{
    public:
        Flag( ShortName short_name_, LongName long_name_, Description description_)
        :description(std::move(description_))
        ,short_name(std::move(short_name_))
        ,longName(std::move(long_name_)){}

        auto value()const -> CheckedCmdTypesafe::Typesafe<bool, TypeTag>{
            return CheckedCmdTypesafe::Typesafe<bool, TypeTag>(boolval);
        }

        auto GetShortName() const -> ShortName const&{
            return short_name;
        }

        auto GetName() const -> LongName const&{
            return longName;
        }

        auto GetDescription() const -> Description const&{
            return description;
        }

        bool IsInValid()const{
            return false;
        }
    private:
        friend struct ClaraFlagBinder<CheckedCmdTypesafe::Typesafe<bool, TypeTag>>;
        Description description;
        ShortName short_name;
        LongName longName;
        bool boolval = false;

    };

    class ParamBase{
    public:
        ParamBase(Hint hint_, ShortName short_name_, LongName long_name_, Description description_)
            :description(std::move(description_))
             ,short_name(std::move(short_name_))
             ,long_name(std::move(long_name_))
             ,hint(std::move(hint_))
             {}
        auto GetShortName() const -> ShortName const&{
            return short_name;
        }

        auto GetName() const -> LongName const&{
            return long_name;
        }

        auto GetHint() const -> Hint const&{
            return hint;
        }

        auto GetDescription() const -> Description const&{
            return description;
        }
    protected:
        Description description;
        ShortName short_name;
        LongName long_name;
        Hint hint;
    };

    template<typename Type>
    class Param;

    template<typename Type>
    class Param: public ParamBase {
    public:
        Param(Hint hint_, ShortName short_name_, LongName long_name_, Description description_, std::function<bool(Type)> pred)
                :ParamBase(std::move(hint_)
                           ,std::move(short_name_)
                           ,std::move(long_name_)
                           ,std::move(description_)
                          )
                 ,predicate(pred)
                 {}

        auto value()const -> Type const&{
            return val;
        }

        bool IsInValid()const{
            return !predicate(val);
        }

    private:
        friend struct ClaraParamBinder<Type>;
        Type val;
        std::function<bool(Type)> predicate;
    };

    template<typename Type>
    class Arg{
    public:
        Arg(Hint hint_, Description description_, std::function<bool(Type)> pred)
                :description(std::move(description_))
                 ,hint(std::move(hint_))
                 ,predicate(pred)
                 {}

        auto value()const -> Type const&{
            return val;
        }

        auto GetHint() const -> Hint const&{
            return hint;
        }

        auto GetDescription() const -> Description const&{
            return description;
        }

        bool IsInValid()const{
            return !predicate(val);
        }
    private:
        friend struct ClaraArgBinder<Type>;
        Type val;
        Description description;
        Hint hint;
        std::function<bool(Type)> predicate;
    };

    using Help = CheckedCmd::Flag<HelpFlag>;
    template<typename T>
    struct ClaraArgBinder{
        static clara::Parser bind(clara::Parser const& parser, Arg<T>& t){
            return parser|clara::Arg(t.val,t.GetHint().Get())(t.GetDescription().Get());
        }
    };

    template<typename T>
    struct ClaraParamBinder{
        static clara::Parser bind(clara::Parser const& parser, Param<T>& t){
            return parser|clara::Opt(t.val,t.GetHint().Get())
                          [t.GetShortName().Get()][t.GetName().Get()]
                          (t.GetDescription().Get()).required();
        }
    };

    template<>
    class Flag<HelpFlag>{
    public:
        auto value()const -> bool{
            return helpval;
        }
        auto GetShortName() const -> ShortName const&{
            return short_name;
        }

        auto GetName() const -> LongName const&{
            return longName;
        }

        auto GetDescription() const -> Description const&{
            return description;
        }

        void SetDescription(Description desc){
            description = std::move(desc);
        }

        bool IsInValid() const{
            return false;
        };
    private:
        friend struct ClaraFlagBinder<HelpFlag>;
        Description description = Description("");
        ShortName short_name = ShortName("-h");
        LongName longName = LongName("--help");
        bool helpval=false;
    };

    template<>
    struct ClaraFlagBinder<HelpFlag>{
        static clara::Parser bind(clara::Parser const& parser, Flag<HelpFlag>& t){
            return parser|clara::Opt(t.helpval)
            [t.GetShortName().Get()][t.GetName().Get()]
                    (t.GetDescription().Get());
        }
    };


    template<typename Type>
    class Param<std::optional< Type>>: public ParamBase{
    public:
        Param( Hint hint_, ShortName short_name_, LongName long_name_, Description description_, std::function<bool(Type)> pred)
                :ParamBase(std::move(hint_)
                           ,std::move(short_name_)
                           ,std::move(long_name_)
                           ,std::move(description_)
                          )
                 ,predicate(pred)
                 {}

        auto value_or(Type val_)const -> Type{
            return (val.has_value())?val.value():val_;
        }

        bool IsInValid()const{
            return val.has_value() && !predicate(val.value());
        };
    private:
        friend struct ClaraParamBinder<std::optional<Type>>;
        std::optional< Type> val;
        std::function<bool(Type)> predicate;
    };

    template<typename TypeTag>
    class Param<std::optional< CheckedCmdTypesafe::Typesafe<char, TypeTag>>>: public ParamBase{
    public:
        Param( Hint hint_, ShortName short_name_, LongName long_name_, Description description_, std::function<bool(char)> pred)
                :ParamBase(std::move(hint_)
                ,std::move(short_name_)
                ,std::move(long_name_)
                ,std::move(description_)
        )
                 ,predicate(pred)
        {}

        auto value_or(CheckedCmdTypesafe::Typesafe<char, TypeTag> val_)const -> CheckedCmdTypesafe::Typesafe<char, TypeTag>{
            if(val.has_value()) {
                if (std::regex_match(val.value(), std::regex(R"(^.$)"))) {
                    return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val.value()[0]);
                }
                if (std::regex_match(val.value(), std::regex(R"(^'.'$)"))) {
                    return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val.value()[1]);
                }
                if (std::regex_match(val.value(), std::regex(R"(^"."$)"))) {
                    return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val.value()[1]);
                }
            }
            return val_;
        }

        bool IsInValid()const{
            if (!val.has_value()) return false;

            if(std::regex_match(val.value(), std::regex(R"(^.$)"))){
                return !predicate(val.value()[0]);
            }
            if(std::regex_match(val.value(), std::regex(R"(^'.'$)"))){
                return !predicate(val.value()[1]);
            }
            if (std::regex_match(val.value(), std::regex(R"(^"."$)"))) {
                return !predicate(val.value()[1]);
            }
            return true;
        }
    private:
        friend  struct ClaraParamBinder<std::optional<CheckedCmdTypesafe::Typesafe<char, TypeTag>>>;
        std::optional< std::string> val;
        std::function<bool(char)> predicate;
    };

    template<typename TypeTag>
    class Param<CheckedCmdTypesafe::Typesafe<char, TypeTag>>: public ParamBase{
    public:
        Param( Hint hint_, ShortName short_name_, LongName long_name_, Description description_, std::function<bool(char)> pred)
                :ParamBase(std::move(hint_)
                ,std::move(short_name_)
                ,std::move(long_name_)
                ,std::move(description_)
        )
                 ,predicate(pred)
        {}

        auto value()const -> CheckedCmdTypesafe::Typesafe<char, TypeTag>{
            if (std::regex_match(val, std::regex(R"(^'.'$)"))) {
                return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val[1]);
            }
            if (std::regex_match(val, std::regex(R"(^"."$)"))) {
                return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val[1]);
            }
            return CheckedCmdTypesafe::Typesafe<char, TypeTag>(val[0]);
        }

        bool IsInValid()const{
            if(std::regex_match(val, std::regex(R"(^.$)"))){
                return !predicate(val[0]);
            }
            if(std::regex_match(val, std::regex(R"(^'.'$)"))){
                return !predicate(val[1]);
            }
            if (std::regex_match(val, std::regex(R"(^"."$)"))) {
                return !predicate(val[1]);
            }
            return true;
        }
    private:
        friend  struct ClaraParamBinder<CheckedCmdTypesafe::Typesafe<char, TypeTag>>;
        std::string val;
        std::function<bool(char)> predicate;
    };

    template<typename TypeTag>
    struct ClaraParamBinder<std::optional<CheckedCmdTypesafe::Typesafe<char, TypeTag>>>{
        static clara::Parser bind(clara::Parser const& parser, Param<std::optional<CheckedCmdTypesafe::Typesafe<char, TypeTag>>>& t){
            return parser|clara::Opt(t.val,t.GetHint().Get())
            [t.GetShortName().Get()][t.GetName().Get()]
                    (t.GetDescription().Get());
        }
    };

    template<typename TypeTag>
    struct ClaraParamBinder<CheckedCmdTypesafe::Typesafe<char, TypeTag>>{
        static clara::Parser bind(clara::Parser const& parser, Param<CheckedCmdTypesafe::Typesafe<char, TypeTag>>& t){
            return parser|clara::Opt(t.val,t.GetHint().Get())
            [t.GetShortName().Get()][t.GetName().Get()]
                    (t.GetDescription().Get()).required();
        }
    };

    template<typename Type>
    struct ClaraParamBinder<std::optional<Type>>{
        static clara::Parser bind(clara::Parser const& parser, Param<std::optional<Type>>& t){
            return parser|clara::Opt(t.val,t.GetHint().Get())
            [t.GetShortName().Get()][t.GetName().Get()]
                    (t.GetDescription().Get());
        }
    };

    template<typename Type>
    clara::Parser BindToClaraArg(clara::Parser const& parser, Arg<Type>& t){
        return ClaraArgBinder<Type>::bind(parser, t);
    }
    template<typename Type>
    clara::Parser BindToClaraArg(clara::Parser const& parser, Param<Type>& t){
        return ClaraParamBinder<Type>::bind(parser, t);
    }
    template<typename TypeTag>
    clara::Parser BindToClaraArg(clara::Parser const& parser, Flag<CheckedCmdTypesafe::Typesafe<bool,TypeTag>>& t){
        return ClaraFlagBinder<CheckedCmdTypesafe::Typesafe<bool,TypeTag>>::bind(parser, t);
    }

    template<std::size_t> struct int2type {};

    template<typename TUPLE, std::size_t INDEX>
    auto BindToClaraImpl(clara::Parser const& parser, TUPLE& tu,int2type<INDEX>){
        return BindToClaraImpl(BindToClaraArg(parser, std::get<INDEX>(tu)), tu,int2type<INDEX -1>() );
    }
    template<typename TUPLE>
    auto BindToClaraImpl(clara::Parser const& parser, TUPLE& tu,int2type<0>){
        return BindToClaraArg(parser, std::get<0>(tu));
    }
    template<typename TUPLE>
    auto BindToClara(TUPLE& tu){
        return BindToClaraImpl(clara::Parser(),tu, int2type<std::tuple_size<TUPLE>::value - 1>());
    }

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T, typename... Us>
    struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

    template<typename Tuple>
    constexpr bool IsInvalid(Tuple const& tuple){
        constexpr auto is_invalid = [](auto const&...  args) {
            return (... || args.IsInValid());
        };
        return std::apply(is_invalid, tuple);
    }

    auto const IsParseError = [](auto const& parser_result){
        return !(parser_result);
    };

    template<typename ARG_TUPLE>
    auto ParseCmdArgsTuple(int argc, char**argv , ARG_TUPLE arg)-> std::optional<ARG_TUPLE>{
        auto const parser = BindToClara(arg);
        auto const result = parser.parse(clara::Args(argc, argv));

        if (IsParseError(result) || IsInvalid(arg))
            return std::nullopt;

        if constexpr (has_type<Help, ARG_TUPLE>::value){
            if(std::get<Help>(arg).value()){
                std::ostringstream oss;
                oss << parser;
                std::get<Help>(arg).SetDescription(Description(oss.str()));
            }
        }
        return arg;
    }

    template<typename ... ARGS>
    auto ParseCmd(int argc, char**argv, ARGS&& ... args){
        return ParseCmdArgsTuple(argc, argv, Helpers::reverse(std::make_tuple( args ...)));
    }
}

#endif //CHECKED_CMD_H
