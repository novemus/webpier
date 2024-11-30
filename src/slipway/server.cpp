#include <boost/asio.hpp>
#include <boost/process/async_pipe.hpp>
#include <iostream>

boost::process::async_pipe open_pipe(boost::asio::io_context& io, const char* source, const char* sink)
{
    if (mkfifo(source, 0600) && errno != EEXIST)
        boost::process::detail::throw_last_error();

    if (mkfifo(sink, 0600) && errno != EEXIST)
        boost::process::detail::throw_last_error();

    int read_fd = open(source, O_RDWR);
    if (read_fd == -1)
        boost::process::detail::throw_last_error();

    int write_fd = open(sink, O_RDWR);
    if (read_fd == -1)
        boost::process::detail::throw_last_error();

    boost::process::detail::posix::basic_pipe<char*> basic(read_fd, write_fd);
    boost::process::async_pipe pipe(io, basic);

    basic.assign_sink(-1);
    basic.assign_source(-1);

    return pipe;
}

int main(int argc, char* argv[])
{
    std::cout << "in: " << argv[1] << " out: " << argv[2] << " msg: " << argv[3] << std::endl;

    boost::asio::io_context io;

    auto pipe = open_pipe(io, argv[1], argv[2]);

    std::string msg = argv[3];
    msg.push_back('\0');

    boost::asio::async_write(pipe, boost::asio::buffer(msg.data(), msg.size()), 
        [&](const boost::system::error_code& ec, std::size_t size)
        {
            if (!ec)
            {
                std::cout << "written: " << argv[3] << " (" << size << ")" << std::endl;
            }
            else
            {
                std::cerr << "error: " << ec.message() << std::endl;
            }
        });

    boost::asio::streambuf buffer;
    boost::asio::async_read_until(pipe, buffer, '\0',
        [&buffer](const boost::system::error_code& ec, std::size_t size)
        {
            if (!ec)
            {
                std::cout << "read: " << std::string((char*)buffer.data().data(), size) << " (" << size << ")" << std::endl;
            }
            else
            {
                std::cerr << "error: " << ec.message() << std::endl;
            }
        });
    io.run();

    return 0;
}
