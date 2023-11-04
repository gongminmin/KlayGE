include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
	#include <charconv>
	#include <cstring>

	int main()
	{
		char const* str = \"42\";
		int int_val;
		[[maybe_unused]] std::from_chars_result result = std::from_chars(str, str + std::strlen(str), int_val);
	}"
	KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
)

check_cxx_source_compiles("
	#include <charconv>
	#include <cstring>

	int main()
	{
		char const* str = \"42.1\";
		float float_val;
		[[maybe_unused]] std::from_chars_result result = std::from_chars(str, str + std::strlen(str), float_val);
	}"
	KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT
)

check_cxx_source_compiles("
	#include <bit>

	int main()
	{
		float f = 10;
		[[maybe_unused]] int i = std::bit_cast<int>(f);
	}"
	KLAYGE_CXX20_LIBRARY_BIT_CAST_SUPPORT
)

check_cxx_source_compiles("
	#include <bit>

	int main()
	{
		unsigned int ui = 10;
		[[maybe_unused]] int ret = std::countl_zero(ui);
	}"
	KLAYGE_CXX20_LIBRARY_BIT_OPERATIONS_SUPPORT
)

check_cxx_source_compiles("
	#include <bit>

	int main()
	{
		[[maybe_unused]] std::endian l = std::endian::little;
		[[maybe_unused]] std::endian b = std::endian::big;
		[[maybe_unused]] std::endian n = std::endian::native;
	}"
	KLAYGE_CXX20_LIBRARY_ENDIAN_SUPPORT
)

check_cxx_source_compiles("
	#include <bit>

	int main()
	{
		[[maybe_unused]] unsigned int c = std::bit_ceil(42u);
	}"
	KLAYGE_CXX20_LIBRARY_INTEGRAL_POWER_OF_2_OPERATIONS_SUPPORT
)

check_cxx_source_compiles("
	#include <format>
	#include <string>

	int main()
	{
		[[maybe_unused]] std::string str = std::format(\"{} {}\", 1, 2.0f);
	}"
	KLAYGE_CXX20_LIBRARY_FORMAT_SUPPORT
)

check_cxx_source_compiles("
	#include <span>

	int main()
	{
		int arr[] = {1, 2, 3};
		[[maybe_unused]] std::span<int const> s(arr, arr + sizeof(arr) / sizeof(arr[0]));
	}"
	KLAYGE_CXX20_LIBRARY_SPAN_SUPPORT
)

check_cxx_source_compiles("
	#include <utility>

	int main()
	{
		enum class e : int
		{
			One,
			Two,
		};

		[[maybe_unused]] int i = std::to_underlying(e::One);
	}"
	KLAYGE_CXX23_LIBRARY_TO_UNDERLYING_SUPPORT
)

check_cxx_source_compiles("
	#include <utility>

	int main()
	{
		std::unreachable();
	}"
	KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT
)

configure_file(
	${KLAYGE_ROOT_DIR}/KFL/include/KFL/Config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/KFL/include/KFL/Config.hpp
)
