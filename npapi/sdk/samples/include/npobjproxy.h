
#pragma once

#include <pluginbase.h>

class NPVariantProxy : public NPVariant
{
public:
	NPVariantProxy() {
		VOID_TO_NPVARIANT(*this);
	}
	NPVariantProxy(NPVariant npv) {	
		VOID_TO_NPVARIANT(*this);
		this->type = npv.type;
		this->value = npv.value;
	}
	~NPVariantProxy() {
		NPN_ReleaseVariantValue(this);
	}
};

class NPObjectProxy
{
public:
	NPObjectProxy() {
		m_obj = NULL;
	}
	NPObjectProxy(NPObject* obj) {
		m_obj = obj;
	}
	void retain() {
		if( m_obj ) {
			NPN_RetainObject(m_obj);
		}
	}
	void reset() {
		if( m_obj ) {
			NPN_ReleaseObject(m_obj);
		}
		m_obj = NULL;
	}
	~NPObjectProxy() {
		reset();
	}
	operator NPObject* &(){
		return m_obj;
	}
	bool operator !(){
		return !m_obj;
	}
	NPObjectProxy& operator =(NPObject* obj) {
		reset();
		m_obj = obj;
		return *this;
	}
private:
	NPObject* m_obj; 
};