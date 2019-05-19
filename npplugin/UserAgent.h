#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#define UA_CHROME	L"chrome"
#define UA_FIREFOX	L"firefox"
#define UA_OPERA	L"opera"
#define UA_SAFARI	L"safari"
#define UA_LIEBAO	L"lbbrowser"
#define UA_TAOBAO	L"taobrowser"
#define UA_QQ		L"qqbrowser"
#define UA_360SE	L"360se"

struct CUserAgent
{
	CUserAgent()
	{
		m_vecUserAgent.push_back(UA_CHROME);
		m_vecUserAgent.push_back(UA_FIREFOX);
		m_vecUserAgent.push_back(UA_OPERA);
		m_vecUserAgent.push_back(UA_SAFARI);
		m_vecUserAgent.push_back(UA_LIEBAO);
		m_vecUserAgent.push_back(UA_TAOBAO);
		m_vecUserAgent.push_back(UA_QQ);
		m_vecUserAgent.push_back(UA_360SE);
	}

	struct FindUA
	{
		FindUA(const std::wstring strUserAgent)
		{
			m_strUserAgent = strUserAgent;
		}

		bool operator()(const std::wstring& strUA) const
		{
			std::wstring::size_type nPos = m_strUserAgent.find(strUA);
			if( nPos != std::wstring::npos )
			{
				return true;
			}
			return false;
		}
	private:
		std::wstring m_strUserAgent;
	};
	
	bool Navigator(std::wstring& strUserAgent)
	{
		std::vector<std::wstring>::const_iterator pit;
		pit = std::find_if(m_vecUserAgent.begin(), m_vecUserAgent.end(), FindUA(strUserAgent));
		if( pit != m_vecUserAgent.end() )
		{
			strUserAgent = *pit;
			return true;
		}
		return false;
	}

private:
	std::vector<std::wstring> m_vecUserAgent;
};