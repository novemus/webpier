#include <backend/message.h>
#include <store/utils.h>
#include <boost/property_tree/json_parser.hpp>

namespace slipway
{
    namespace
    {
        boost::property_tree::ptree convert_handle(const slipway::handle& obj) noexcept(true)
        {
            boost::property_tree::ptree doc;
            doc.put("pier", webpier::locale_to_utf8(obj.pier));
            doc.put("service", webpier::locale_to_utf8(obj.service));
            return doc;
        }

        slipway::handle convert_handle(const boost::property_tree::ptree& doc) noexcept(false)
        {
            slipway::handle obj;
            obj.pier = webpier::utf8_to_locale(doc.get<std::string>("pier"));
            obj.service = webpier::utf8_to_locale(doc.get<std::string>("service"));
            return obj;
        }

        boost::property_tree::ptree convert_health(const slipway::health& obj) noexcept(true)
        {
            boost::property_tree::ptree doc;
            doc.put("pier", webpier::locale_to_utf8(obj.pier));
            doc.put("service", webpier::locale_to_utf8(obj.service));
            doc.put("state", obj.state);
            doc.put("message", webpier::locale_to_utf8(obj.message));
            return doc;
        }

        slipway::health convert_health(const boost::property_tree::ptree& doc) noexcept(false)
        {
            slipway::health obj;
            obj.pier = webpier::utf8_to_locale(doc.get<std::string>("pier"));
            obj.service = webpier::utf8_to_locale(doc.get<std::string>("service"));
            obj.state = static_cast<health::status>(doc.get<int>("state"));
            obj.message = webpier::utf8_to_locale(doc.get<std::string>("message"));
            return obj;
        }

        boost::property_tree::ptree convert_report(const slipway::report& obj) noexcept(true)
        {
            boost::property_tree::ptree doc;
            doc.put("pier", webpier::locale_to_utf8(obj.pier));
            doc.put("service", webpier::locale_to_utf8(obj.service));
            doc.put("state", obj.state);
            doc.put("message", webpier::locale_to_utf8(obj.message));

            boost::property_tree::ptree context;
            for(const auto& link : obj.tunnels)
            {
                boost::property_tree::ptree item;
                item.put("pid", link.pid);
                item.put("pier", webpier::locale_to_utf8(link.pier));
                context.push_back(std::make_pair("", item));
            }
            doc.put_child("tunnels", context);
            return doc;
        }

        slipway::report convert_report(const boost::property_tree::ptree& doc) noexcept(false)
        {
            slipway::report obj;
            obj.pier = webpier::utf8_to_locale(doc.get<std::string>("pier"));
            obj.service = webpier::utf8_to_locale(doc.get<std::string>("service"));
            obj.state = static_cast<health::status>(doc.get<int>("state"));
            obj.message = webpier::utf8_to_locale(doc.get<std::string>("message"));

            boost::property_tree::ptree tunnels;
            for (auto& item : doc.get_child("tunnels", tunnels))
            {
                report::tunnel tunnel;
                tunnel.pid = item.second.get<int>("pid");
                tunnel.pier = webpier::utf8_to_locale(item.second.get<std::string>("pier"));
                obj.tunnels.emplace_back(std::move(tunnel));
            }
            return obj;
        }
    }

    void push_message(std::streambuf& buffer, const slipway::message& msg) noexcept(true)
    {
        boost::property_tree::ptree doc;

        doc.put("action", msg.action);
        switch(msg.payload.index())
        {
            case 0:
            {
                const auto& error = std::get<std::string>(msg.payload);
                if (!error.empty())
                    doc.put("error", webpier::locale_to_utf8(error));
                break;
            }
            case 1:
            {
                doc.put_child("handle", convert_handle(std::get<slipway::handle>(msg.payload)));
                break;
            }
            case 2:
            {
                doc.put_child("health", convert_health(std::get<slipway::health>(msg.payload)));
                break;
            }
            case 3:
            {
                doc.put_child("report", convert_report(std::get<slipway::report>(msg.payload)));
                break;
            }
            case 4:
            {
                boost::property_tree::ptree health;
                for (const auto& item : std::get<std::vector<slipway::health>>(msg.payload))
                    health.push_back(std::make_pair("", convert_health(item)));
                doc.put_child("health", health);
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
#if defined(WIN32) || defined(__APPLE__)
        stream << '\n';
#endif
    }

    void pull_message(std::streambuf& buffer, slipway::message& msg) noexcept(false)
    {
        std::istream stream(&buffer);
        boost::property_tree::ptree doc;
        boost::property_tree::read_json(stream, doc);

        msg.action = static_cast<message::command>(doc.get<int>("action"));
        msg.payload = {};

        if (doc.count("error"))
        {
            msg.payload = webpier::utf8_to_locale(doc.get<std::string>("error"));
        }
        else if (doc.count("handle"))
        {
            msg.payload = convert_handle(doc.get_child("handle"));
        }
        else if (doc.count("health"))
        {
            boost::property_tree::ptree health;
            health = doc.get_child("health", health);
            if (health.count("") || health.empty())
            {
                std::vector<slipway::health> list;
                for (const auto& item : health)
                    list.emplace_back(convert_health(item.second));
                msg.payload = list;
            }
            else
            {
                msg.payload = convert_health(health);
            }
        }
        else if (doc.count("report"))
        {
            boost::property_tree::ptree report;
            report = doc.get_child("report", report);
            if (report.count("") || report.empty())
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
}
