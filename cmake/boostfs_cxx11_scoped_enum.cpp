
#include <boost/filesystem/operations.hpp>

namespace bfs = boost::filesystem;

int main() {
	BOOST_SCOPED_ENUM(bfs::copy_option) opt = bfs::copy_option::overwrite_if_exists;
	bfs::copy_file("a", "b",);
	return 0;
}
