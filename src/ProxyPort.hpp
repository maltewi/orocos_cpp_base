#ifndef PROXYPORT_H
#define PROXYPORT_H

#include <rtt/FlowStatus.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>
#include "OrocosHelpers.hpp"
#include <rtt/TaskContext.hpp>

template<typename T>
class OutputProxyPort;

template<typename T>
class InputProxyPort;

class ProxyPortBase
{
protected:
    std::string getFreePortName(RTT::TaskContext *clientTask, RTT::base::PortInterface *portIf);
};

template<typename T>
class InputProxyPort : public ProxyPortBase
{
    friend class OutputProxyPort<T>;
    RTT::base::InputPortInterface *port;
    RTT::OutputPort<T> *writer;
public:
    InputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::InputPortInterface *>(iface)), writer(NULL)
    {
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };

    RTT::base::InputPortInterface *getPortInterface()
    {
        return port;
    }
    
    RTT::OutputPort<T> &getWriter()
    {
        return getWriter(port->getDefaultPolicy());
    }
    
    RTT::OutputPort<T> &getWriter(RTT::ConnPolicy const& policy)
    {
        if(!writer)
        {
            writer = dynamic_cast<RTT::OutputPort<T> * >(port->antiClone());
            if(!writer)
                throw std::runtime_error("Error, could not get writer for port " + port->getName()); 

            RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
            writer->setName(getFreePortName(clientTask, port));
            clientTask->addPort(*writer);
            if(!port->connectTo(writer, policy))
                throw std::runtime_error("InputProxyPort::getWriter(): Error could not connect writer to port " + port->getName() + " of task " + port->getInterface()->getOwner()->getName());
        }
        return *writer;
    };

    void removeWriter()
    {
        if(writer)
        {
            writer->disconnect();
            delete writer;
            writer = NULL;
        }
    }

    bool disconnect()
    {
        port->disconnect();
        return true;
    }

    bool disconnect(OutputProxyPort<T> &outPut)
    {
        port->disconnect(&outPut);
        return  true;
    }
    
    ~InputProxyPort()
    {
        removeWriter();
    }
};

template<typename T>
class OutputProxyPort : public ProxyPortBase
{
    friend class InputProxyPort<T>;
    RTT::base::OutputPortInterface *port;
    RTT::InputPort<T> *reader;
public:
    OutputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::OutputPortInterface *>(iface)), reader(NULL)
    {
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };

    RTT::base::OutputPortInterface *getPortInterface()
    {
        return port;
    }
        RTT::InputPort<T> &getReader()
    {
        return getReader(RTT::ConnPolicy());
    }

    RTT::InputPort<T> &getReader(RTT::ConnPolicy const& policy)
    {
        if(!reader)
        {
            reader = dynamic_cast<RTT::InputPort<T> *>(port->antiClone());
            if(!reader)
                throw std::runtime_error("Error, could not get reader for port " + port->getName()); 
            RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
            reader->setName(getFreePortName(clientTask, port));
            clientTask->addPort(*reader);
            if(!reader->connectTo(port, policy))
                throw std::runtime_error("InputProxyPort::getReader(): Error could not connect reader to port " + port->getName() + " of task " + port->getInterface()->getOwner()->getName());
        }        
        return *reader;
    };    

    void deleteReader()
    {
        if(reader)
        {
            reader->disconnect();
            delete reader;
            reader = NULL;
        }
    }
    
    bool connectTo(InputProxyPort<T> &inputPort, RTT::ConnPolicy const& policy)
    {
        return port->connectTo(inputPort.port, policy);
    };
    
    bool connectTo(InputProxyPort<T> &inputPort)
    {
        return port->connectTo(inputPort.port);
    };
    
    bool disconnect()
    {
        port->disconnect();
        return true;
    }

    bool disconnect(InputProxyPort<T> &inputPort)
    {
        port->disconnect(&inputPort);
        return true;
    }

    ~OutputProxyPort()
    {
        deleteReader();
    }
};


#endif // PROXYPORT_H
