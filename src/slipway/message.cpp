#include "message.h"
#include <boost/property_tree/json_parser.hpp>

namespace slipway
{
    boost::property_tree::ptree convert_handle(const handle& obj) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("node", obj.node);
        doc.put("service", obj.service);
        return doc;
    }

    handle convert_handle(const boost::property_tree::ptree& doc) noexcept(false)
    {
        handle obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        return obj;
    }

    boost::property_tree::ptree convert_outline(const outline& obj) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("node", obj.node);
        doc.put("service", obj.service);
        doc.put("state", obj.state);
        return doc;
    }

    outline convert_outline(const boost::property_tree::ptree& doc) noexcept(false)
    {
        outline obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        obj.state = static_cast<outline::status>(doc.get<int>("state"));
        return obj;
    }

    boost::property_tree::ptree convert_snapshot(const snapshot& obj) noexcept(true)
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

    snapshot convert_snapshot(const boost::property_tree::ptree& doc) noexcept(false)
    {
        snapshot obj;
        obj.node = doc.get<std::string>("node");
        obj.service = doc.get<std::string>("service");
        obj.state = static_cast<outline::status>(doc.get<int>("state"));
        obj.logfile = doc.get<std::string>("logfile");

        boost::property_tree::ptree context;
        for (auto& item : doc.get_child("context", context))
        {
            snapshot::linkage link;
            link.pid = item.second.get<int>("pid");
            link.peer = item.second.get<std::string>("peer");
            link.logfile = item.second.get<std::string>("logfile");
            obj.context.emplace_back(std::move(link));
        }
        return obj;
    }

    void put_message(boost::asio::streambuf& buffer, const message& msg) noexcept(true)
    {
        boost::property_tree::ptree doc;
        doc.put("action", msg.action);

        switch(msg.payload.index())
        {
            case 0:
            {
                const auto& error = std::get<0>(msg.payload);
                if (!error.empty())
                    doc.put("error", error);
                break;
            }
            case 1:
            {
                doc.put_child("handle", convert_handle(std::get<1>(msg.payload)));
                break;
            }
            case 2:
            {
                doc.put_child("outline", convert_handle(std::get<2>(msg.payload)));
                break;
            }
            case 3:
            {
                doc.put_child("snapshot", convert_snapshot(std::get<3>(msg.payload)));
                break;
            }
            case 4:
            {
                boost::property_tree::ptree outline;
                for (const auto& item : std::get<4>(msg.payload))
                    outline.push_back(std::make_pair("", convert_outline(item)));
                doc.put_child("outline", outline);
                break;
            }
            case 5:
            {
                boost::property_tree::ptree snapshot;
                for (const auto& item : std::get<5>(msg.payload))
                    snapshot.push_back(std::make_pair("", convert_snapshot(item)));
                doc.put_child("snapshot", snapshot);
                break;
            }
            default:
                break;
        }

        std::ostream stream(&buffer);
        boost::property_tree::write_json(stream, doc);
        stream << '\0';
    }

    void get_message(boost::asio::streambuf& buffer, message& msg) noexcept(false)
    {
        std::istream stream(&buffer);
        boost::property_tree::ptree doc;
        boost::property_tree::read_json(stream, doc);

        msg.action = static_cast<message::command>(doc.get<int>("action"));
        if (doc.count("error"))
        {
            msg.payload = doc.get<std::string>("error");
        }
        else if (doc.count("handle"))
        {
            msg.payload = convert_handle(doc.get_child("handle"));
        }
        else if (doc.count("outline"))
        {
            boost::property_tree::ptree outline;
            doc.get_child("outline", outline);
            if (outline.count(""))
            {
                std::vector<slipway::outline> list;
                for (const auto& item : outline)
                    list.emplace_back(convert_outline(item.second));
                msg.payload = list;
            }
            else
            {
                msg.payload = convert_outline(outline);
            }
        }
        else if (doc.count("snapshot"))
        {
            boost::property_tree::ptree snapshot;
            doc.get_child("snapshot", snapshot);
            if (snapshot.count(""))
            {
                std::vector<slipway::snapshot> list;
                for (const auto& item : snapshot)
                    list.emplace_back(convert_snapshot(item.second));
                msg.payload = list;
            }
            else
            {
                msg.payload = convert_snapshot(snapshot);
            }
        }
    }
}
