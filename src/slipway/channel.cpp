#include "channel.h"
#include <iostream>
#include <boost/asio/spawn.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace slipway
{
    boost::property_tree::ptree convert_handle(const slipway::handle& obj) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("node", obj.node);
        doc.put("service", obj.service);
        return doc;
    }

    slipway::handle convert_handle(const boost::property_tree::ptree& doc) noexcept(false)
    {
        slipway::handle obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        return obj;
    }

    boost::property_tree::ptree convert_wealth(const slipway::wealth& obj) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("node", obj.node);
        doc.put("service", obj.service);
        doc.put("state", obj.state);
        return doc;
    }

    slipway::wealth convert_wealth(const boost::property_tree::ptree& doc) noexcept(false)
    {
        slipway::wealth obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        obj.state = static_cast<wealth::status>(doc.get<int>("state"));
        return obj;
    }

    boost::property_tree::ptree convert_report(const slipway::report& obj) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("node", obj.node);
        doc.put("service", obj.service);
        doc.put("state", obj.state);
        doc.put("logfile", obj.logfile);

        boost::property_tree::ptree context;
        for(const auto& link : obj.context)
        {
            boost::property_tree::ptree item;
            item.put("pid", link.pid);
            item.put("peer", link.peer);
            item.put("logfile", link.logfile);
            context.push_back(std::make_pair("", item));
        }
        doc.put_child("context", context);
        return doc;
    }

    slipway::report convert_report(const boost::property_tree::ptree& doc) noexcept(false)
    {
        slipway::report obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        obj.state = static_cast<wealth::status>(doc.get<int>("state"));
        obj.logfile = doc.get<std::string>("logfile");

        boost::property_tree::ptree context;
        for (auto& item : doc.get_child("context", context))
        {
            report::linkage link;
            link.pid = item.second.get<int>("pid");
            link.peer = item.second.get<std::string>("peer");
            link.logfile = item.second.get<std::string>("logfile");
            obj.context.emplace_back(std::move(link));
        }
        return obj;
    }

    void put_message(boost::asio::streambuf& buffer, const slipway::message& msg) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("action", msg.action);

        switch(msg.payload.index())
        {
            case 0:
            {
                const auto& error = std::get<std::string>(msg.payload);
                if (!error.empty())
                    doc.put("error", error);
                break;
            }
            case 1:
            {
                doc.put_child("handle", convert_handle(std::get<slipway::handle>(msg.payload)));
                break;
            }
            case 2:
            {
                doc.put_child("wealth", convert_wealth(std::get<slipway::wealth>(msg.payload)));
                break;
            }
            case 3:
            {
                doc.put_child("report", convert_report(std::get<slipway::report>(msg.payload)));
                break;
            }
            case 4:
            {
                boost::property_tree::ptree wealth;
                for (const auto& item : std::get<std::vector<slipway::wealth>>(msg.payload))
                    wealth.push_back(std::make_pair("", convert_wealth(item)));
                doc.put_child("wealth", wealth);
                break;
            }
            case 5:
            {
                boost::property_tree::ptree report;
                for (const auto& item : std::get<std::vector<slipway::report>>(msg.payload))
                    report.push_back(std::make_pair("", convert_report(item)));
                doc.put_child("report", report);
                break;
            }
            default:
                break;
        }

        std::ostream stream(&buffer);
        boost::property_tree::write_json(stream, doc, false);
    }

    void get_message(boost::asio::streambuf& buffer, slipway::message& msg) noexcept(false)
    {
        std::istream stream(&buffer);
        boost::property_tree::ptree doc;
        boost::property_tree::read_json(stream, doc);

        msg.action = static_cast<message::command>(doc.get<int>("action"));
        msg.payload = {};

        if (doc.count("error"))
        {
            msg.payload = doc.get<std::string>("error");
        }
        else if (doc.count("handle"))
        {
            msg.payload = convert_handle(doc.get_child("handle"));
        }
        else if (doc.count("wealth"))
        {
            boost::property_tree::ptree wealth;
            wealth = doc.get_child("wealth", wealth);
            if (wealth.count(""))
            {
                std::vector<slipway::wealth> list;
                for (const auto& item : wealth)
                    list.emplace_back(convert_wealth(item.second));
                msg.payload = list;
            }
            else
            {
                msg.payload = convert_wealth(wealth);
            }
        }
        else if (doc.count("report"))
        {
            boost::property_tree::ptree report;
            report = doc.get_child("report", report);
            if (report.count(""))
            {
                std::vector<slipway::report> list;
                for (const auto& item : report)
                    list.emplace_back(convert_report(item.second));
                msg.payload = list;
            }
            else
            {
                msg.payload = convert_report(report);
            }
        }
    }

    class channel_impl : public channel
    {
        boost::asio::io_context m_io;
        boost::process::async_pipe m_pipe;

#ifdef WIN32
        // TODO: add windows implementation
#else
        static boost::process::async_pipe open_pipe(boost::asio::io_context& io, const std::string& source, const std::string& sink)
        {
            if (mkfifo(source.c_str(), 0600) && errno != EEXIST)
                boost::process::detail::throw_last_error();

            if (mkfifo(sink.c_str(), 0600) && errno != EEXIST)
                boost::process::detail::throw_last_error();

            int read_fd = open(source.c_str(), O_RDWR);
            if (read_fd == -1)
                boost::process::detail::throw_last_error();

            int write_fd = open(sink.c_str(), O_RDWR);
            if (read_fd == -1)
                boost::process::detail::throw_last_error();

            boost::process::detail::posix::basic_pipe<char*> basic(read_fd, write_fd);
            boost::process::async_pipe pipe(io, basic);

            basic.assign_sink(-1);
            basic.assign_source(-1);

            return pipe;
        }
#endif
        void execute(const std::function<void(boost::asio::yield_context yield)>& function, int64_t timeout = 10000) noexcept(false)
        {
            m_io.restart();

            boost::asio::spawn(m_io, [&](boost::asio::yield_context yield)
            {
                boost::asio::deadline_timer timer(m_io);
                if (timeout > 0)
                {
                    timer.expires_from_now(boost::posix_time::milliseconds(timeout));
                    timer.async_wait([&](const boost::system::error_code& error)
                    {
                        if(error)
                        {
                            if (error == boost::asio::error::operation_aborted)
                                return;

                            std::cerr << error.message() << std::endl;
                        }

                        try
                        {
                            m_pipe.cancel();
                        }
                        catch (const std::exception &ex)
                        {
                            std::cout << ex.what() << std::endl;
                        }
                    });
                }

                function(yield);

                boost::system::error_code ec;
                timer.cancel(ec);
            });

            m_io.run();
        }

    public:

        channel_impl(const std::string& source, const std::string& sink)
            : m_pipe(open_pipe(m_io, source, sink))
        {
        }

        void push(const message& request) noexcept(false) override
        {
            execute([&](boost::asio::yield_context yield)
            {
                boost::asio::streambuf buffer;
                put_message(buffer, request);

                boost::system::error_code ec;
                boost::asio::async_write(m_pipe, buffer, yield[ec]);

                if (ec)
                    throw pipe_error(ec.message());
            });
        }

        void pull(message& response) noexcept(false) override
        {
            execute([&](boost::asio::yield_context yield)
            {
                boost::asio::streambuf buffer;

                boost::system::error_code ec;
                boost::asio::async_read_until(m_pipe, buffer, '\n', yield[ec]);

                if (ec)
                    throw pipe_error(ec.message());

                get_message(buffer, response);
            });
        }
    };

    std::shared_ptr<channel> create_channel(const std::string& source, const std::string& sink) noexcept(false)
    {
        return std::make_shared<channel_impl>(source, sink);
    }
}
