#pragma once

//////////////
// ID class //
//////////////

class Id{
private:
	GUID m_guid;
	std::wstring m_string;

public:
	Id();
	~Id();

	std::wstring toString() const;
	const wchar_t *toCString() const;

	bool operator == (const Id &id) const;
	bool operator != (const Id &id) const;
};
