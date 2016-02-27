#include "stdafx.h"

#include "winsock2.h"
#include "upnp.h"
#include "NATUPnP.h"

UPnP::UPnP(void)
{
	m_Port = 0;
}

UPnP::~UPnP(void)
{
	Destroy();
}

bool UPnP::Create(WORD Port)
{
	IUPnPNAT * Nat = NULL;
	IStaticPortMappingCollection * PortMappingCollection = NULL;
	IStaticPortMapping * PortMap = NULL;
	HRESULT Result;
	wchar_t Protocol[256];
	wchar_t InternalClient[256];
	wchar_t Description[256];

	Destroy();

	TRACE("UPnP: Adding port\n");

	if(!GetIp())
	{
		return false;
	}

	mbstowcs(InternalClient, m_Address, 256);
//	swprintf(Protocol, L"TCP");
//	swprintf(Description, L"Torrent");

	swprintf(Protocol, L"UDP");
	swprintf(Description, L"Gunz");

	// Create IUPnPNat
	Result = CoCreateInstance(CLSID_UPnPNAT, NULL, CLSCTX_INPROC_SERVER, IID_IUPnPNAT, (void **)&Nat);
	if(FAILED(Result))
	{
		TRACE("UPnP: Unable to create UPnPNAT interface\n");
		return false;
	}

	Result = Nat->get_StaticPortMappingCollection(&PortMappingCollection);

	if(!PortMappingCollection || FAILED(Result))
	{
		if(PortMappingCollection) PortMappingCollection->Release();
		Nat->Release();

		TRACE("UPnP: Unable to acquire a static portmapping collection\n");
		return false;
	}

	Result = PortMappingCollection->Add(Port, Protocol, Port, InternalClient, VARIANT_TRUE, Description, &PortMap);

	if(!PortMap || FAILED(Result))
	{
		if(PortMap) PortMap->Release();
		PortMappingCollection->Release();
		Nat->Release();

		TRACE("UPnP: Unable add port\n");
		return false;
	}

	TRACE("UPnP: Port %d forwarded to %s\n", Port, m_Address);

	PortMap->Release();
	PortMappingCollection->Release();
	Nat->Release();

	m_Port = Port;

	return true;
}

void  UPnP::Destroy(void)
{
	IUPnPNAT * Nat = NULL;
	IStaticPortMappingCollection * PortMappingCollection = NULL;
	HRESULT Result;
	wchar_t Protocol[256];
	WORD Port;

	if(m_Port == 0) return;
	Port = m_Port;
	m_Port = 0;

	TRACE("UPnP: Removing Port\n");

	swprintf(Protocol, L"TCP");

	// Create IUPnPNat
	Result = CoCreateInstance(CLSID_UPnPNAT, NULL, CLSCTX_INPROC_SERVER, IID_IUPnPNAT, (void **)&Nat);
	if(FAILED(Result))
	{
		TRACE("UPnP: Unable to create UPnPNAT interface\n");
		return;
	}

	Result = Nat->get_StaticPortMappingCollection(&PortMappingCollection);

	if(!PortMappingCollection || FAILED(Result))
	{
		if(PortMappingCollection) PortMappingCollection->Release();
		Nat->Release();

		TRACE("UPnP: Unable to acquire a static portmapping collection\n");
		return;
	}

	Result = PortMappingCollection->Remove(Port, Protocol);

	if(FAILED(Result))
	{
		PortMappingCollection->Release();
		Nat->Release();

		TRACE("UPnP: Unable to remove port\n");
		return;
	}
}

bool UPnP::GetIp(void)
{
	char HostName[256];
	hostent * Host;
	char * Address;
	SOCKADDR_IN Addr;

	int ret;

	// Get your own Ip
	if(ret = gethostname(HostName, 256) != 0)
	{
		TRACE("UPnP: gethostname failed\n");
		return false;
	}else{
		TRACE("UPnP: HostName: %s\n", HostName);
	}
	Host = gethostbyname(HostName);
	if(Host == NULL)
	{
		TRACE("UPnP: gethostbyname failed\n");
		return false;
	}
	Addr.sin_addr.s_addr = ((IN_ADDR *)Host->h_addr)->s_addr;
	Address = inet_ntoa(Addr.sin_addr);

	if(!Address)
	{
		TRACE("UPnP: inet_ntoa failed\n");
		return false;
	}

	strncpy(m_Address, Address, sizeof(m_Address) - 1);

	TRACE("UPnP: Local Ip: %s\n", m_Address);

	return true;
}