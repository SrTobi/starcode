#include "objects/component/filesystem.hpp"

#include <testx/testx.hpp>


TESTX_AUTO_TEST_CASE(test_fs_read_write)
{
    FileSystem fs;

    fs.write("", "root/dir/test.txt", "jscode");
    fs.write("", "root/dir/dummy.txt", "dummy");
    BOOST_CHECK(fs.exists("", "root/dir"));
    BOOST_CHECK(fs.exists("", "root/dir/test.txt"));
    BOOST_CHECK(!fs.exists("", "root/dir/test.txt2"));
    BOOST_CHECK_EQUAL(fs.read("", "root/dir/test.txt"), "jscode");
    BOOST_CHECK_EQUAL(fs.read("root", "dir/test.txt"), "jscode");
    BOOST_CHECK_EQUAL(fs.read("root", "/root/dir/test.txt"), "jscode");
    BOOST_CHECK_EQUAL(fs.read("", "/../root/dir/../dir/test.txt"), "jscode");

    fs.write("", "root/dir/../dir/test.txt", "jscode2");
    BOOST_CHECK(fs.exists("", "root/dir/test.txt"));
    BOOST_CHECK_EQUAL(fs.read("", "root/dir/test.txt"), "jscode2");

    BOOST_CHECK(fs.type("root", "dir") == FSType::Directory);
    BOOST_CHECK(fs.type("root", "dir/test.txt") == FSType::CodeFile);

    fs.erase("", "root/dir/../dir/test.txt");
    BOOST_CHECK(!fs.exists("", "root/dir/test.txt"));
    BOOST_CHECK(fs.exists("", "root/dir/dummy.txt"));
}


TESTX_AUTO_TEST_CASE(test_semver)
{
    FileSystem fs;

    fs.write("", "test-1.1.3.txt", "dummy");
    fs.write("", "test-1.2.3.txt", "dummy");
    fs.write("", "test-1.2.4.txt", "lower");
    fs.write("", "test-1.3.3.txt", "dummy");
    fs.write("", "test-1.3.6.txt", "higest");
    BOOST_CHECK_EQUAL(fs.read("", "test-^.txt"), "higest");
    BOOST_CHECK_EQUAL(fs.read("", "test-1.^.txt"), "higest");
    BOOST_CHECK_EQUAL(fs.read("", "test-1.2.^.txt"), "lower");

    // check overwriting
    fs.write("", "test-1.2^.txt", "lowest?");
    BOOST_CHECK_EQUAL(fs.read("", "test-1.2.4.txt"), "lowest?");

    fs.erase("", "test-1.1^.txt");
    BOOST_CHECK(!fs.exists("", "test-1.1.3.txt"));
    BOOST_CHECK(!fs.erase("", "test-1.1^.txt"));

    BOOST_CHECK(fs.exists("", "test-1.2.3.txt"));
    BOOST_CHECK(fs.exists("", "test-1.2.4.txt"));
    BOOST_CHECK(fs.exists("", "test-1.3.3.txt"));
    BOOST_CHECK(fs.exists("", "test-1.3.6.txt"));
}


TESTX_AUTO_TEST_CASE(test_dirs)
{
    FileSystem fs;

    fs.write("", "root/test.txt", "dummy");
    fs.write("", "root/test2.txt", "dummy");
    fs.write("", "root/dir/test3.txt", "dummy");
    fs.write("", "root2/dir2/test4.txt", "dummy");

    BOOST_CHECK(fs.exists("", "root/test.txt"));
    BOOST_CHECK(fs.exists("", "root/test2.txt"));
    BOOST_CHECK(fs.exists("", "root/dir/test3.txt"));
    BOOST_CHECK(fs.exists("", "root2/dir2/test4.txt"));

    fs.erase("", "root");

    BOOST_CHECK(!fs.exists("", "root/test.txt"));
    BOOST_CHECK(!fs.exists("", "root/test2.txt"));
    BOOST_CHECK(!fs.exists("", "root/dir/test3.txt"));
    BOOST_CHECK(fs.exists("", "root2/dir2/test4.txt"));
}
