#include "reprocurl/api.h"
#include <curl/curl.h>

using namespace prio;
using namespace repro;
 
namespace reprocurl {

void init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}
 
request::request()
	: method_("GET"), url_("/")
{

}

request::request(const Url& url)
	: method_("GET"), url_(url)
{

}

request& request::url(const Url& url)
{
	url_ = url;
	return *this;
}

request& request::method(const std::string& method)
{
	method_ = method;
	return *this;
}

request& request::data(const std::string& payload)
{
	payload_ = payload;
	return *this;
}

request& request::header(const std::string& header, const std::string& value)
{
	headers_.push_back(std::make_pair(header, value));
	return *this;
}

request& request::content_type(const std::string& value)
{
	headers_.push_back(std::make_pair("content-type", value));
	return *this;
}

request& request::accept(const std::string& value)
{
	headers_.push_back(std::make_pair("accept", value));
	return *this;
}

request& request::user_agent(const std::string& value)
{
	headers_.push_back(std::make_pair("user-agent", value));
	return *this;
}


response::response()
{}

response::response(CurlEasy::Ptr c)
	:curl_(c)
{}

long response::status()
{
	return curl_->status();
}

std::string response::content()
{
	return curl_->response_body();
}

std::string response::content_type()
{
	return curl_->response_header("content-type");
}

std::string response::header(const std::string& key)
{
	return curl_->response_header(key);
}

std::vector<std::pair<std::string, std::string>> response::headers()
{
	return curl_->response_headers();
}

Future<response> fetch(request& req)
{
	auto p = repro::promise<response>();

	auto curl = async_curl()
		->method(req.method_)
		->url(req.url_.toString());

	if (!req.payload_.empty())
	{
		curl->data(req.payload_);
	}

	for (auto h : req.headers_)
	{
		curl->header(h.first, h.second);
	}

	curl->perform()
	.then([p](CurlEasy::Ptr curl)
	{
		p.resolve(response(curl));
	})
		.otherwise([p](const std::exception& ex)
	{
		p.reject(ex);
	});

	return p.future();

}

Future<std::vector<response>> fetch_all(const std::vector<request>& requests)
{
	auto p = promise<std::vector<response>>();

	std::shared_ptr<std::vector<response>> responses = std::make_shared<std::vector<response>>();
	std::shared_ptr<int> cnt = std::make_shared<int>( 0 );
	responses->resize(requests.size());

	for( auto r : requests)
	{
		int i = *cnt;
		(*cnt)++;

		fetch(r)
		.then([p,responses,i,cnt](response res)
		{
			(*responses)[i] = res;
			(*cnt)--;

			if( (*cnt) == 0)
			{
				p.resolve(*responses);
			}
		})
		.otherwise([p](const std::exception& ex)
		{
			p.reject(ex);
		});

	}

	return p.future();
}


} // end namespaces