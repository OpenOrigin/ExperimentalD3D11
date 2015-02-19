#include "Engine.h"

Id::Id(){
	wchar_t string[40];

	CoCreateGuid(&m_guid);
	StringFromGUID2(m_guid, string, 40);
	m_string = string;
}

Id::~Id(){

}

std::wstring Id::toString() const{
	return m_string;
}

const wchar_t * Id::toCString() const{
	return m_string.c_str();
}

bool Id::operator== (const Id &id) const{
	return m_guid == id.m_guid;
}

bool Id::operator!= (const Id &id) const{
	return m_guid != id.m_guid;
}
