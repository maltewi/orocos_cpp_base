#pragma once

#include <rtt/FlowStatus.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>
#include "OrocosHelpers.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/extras/ReadOnlyPointer.hpp>

#define FW_DECLARE_TYPE(type) \
    extern template class RTT::InputPort<type>; \
    extern template class RTT::OutputPort<type>; \
    extern template class OutputProxyPort<type>; \
    extern template class InputProxyPort<type>; \
    extern template class RTT::base::ChannelElement<type>;
    
#define INSTANCIATE_TYPE(type) \
    template class OutputProxyPort<type>; \
    template class InputProxyPort<type>;

template<typename T>
class OutputProxyPort;

template<typename T>
class InputProxyPort;

class ProxyPortBase
{
protected:
    RTT::DataFlowInterface df;
    std::string getFreePortName(RTT::TaskContext *clientTask, RTT::base::PortInterface *portIf);
};

template<typename T>
class InputProxyPort : public ProxyPortBase
{
    friend class OutputProxyPort<T>;
    friend class OutputProxyPort< RTT::extras::ReadOnlyPointer<T> >;
    RTT::base::InputPortInterface *port;
    RTT::OutputPort<T> *writer;

public:
    InputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::InputPortInterface *>(iface)), writer(NULL)
    {
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };

    InputProxyPort(RTT::TaskContext *task, const std::string &portName): port(NULL), writer(NULL)
    {
        RTT::base::PortInterface *iface = task->getPort(portName);
        if(!iface)
            throw std::runtime_error("InputProxyPort: Error, task " + task->getName() + " has no port with the name " + portName);

        port = dynamic_cast<RTT::base::InputPortInterface *>(iface);
        if(!port)
            throw std::runtime_error("InputProxyPort: Error, port '" + portName + "' of task '" + task->getName() + "' hast an unexpected type");
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

            df.addPort("writer", *writer);
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
            df.removePort("writer");
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
        port->disconnect(outPut.getPortInterface());
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
    friend class InputProxyPort< RTT::extras::ReadOnlyPointer<T> >;
    RTT::base::OutputPortInterface *port;
    RTT::InputPort<T> *reader;
public:
    OutputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::OutputPortInterface *>(iface)), reader(NULL)
    {
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };

    OutputProxyPort(RTT::TaskContext *task, const std::string &portName): port(NULL), reader(NULL)
    {
        RTT::base::PortInterface *iface = task->getPort(portName);
        if(!iface)
            throw std::runtime_error("OutputProxyPort: Error, task " + task->getName() + " has no port with the name " + portName);

        port = dynamic_cast<RTT::base::OutputPortInterface *>(iface);
        if(!port)
            throw std::runtime_error("OutputProxyPort: Error, port '" + portName + "' of task '" + task->getName() + "' hast an unexpected type");
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

            df.addPort("reader", *reader);
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
            df.removePort("reader");
            reader = NULL;
        }
    }
    
    bool connectTo(InputProxyPort<T> &inputPort, RTT::ConnPolicy const& policy)
    {
        return port->connectTo(inputPort.getPortInterface(), policy);
    }
    
    bool connectTo(InputProxyPort<RTT::extras::ReadOnlyPointer<T> > &inputPort, RTT::ConnPolicy const& policy)
    {
        return port->connectTo(inputPort.getPortInterface(), policy);
    }
    
    bool connectTo(InputProxyPort<T> &inputPort)
    {
        return port->connectTo(inputPort.getPortInterface());
    }


    template<typename T2>
    bool connectTo(InputProxyPort<T2> &inputPort)
    {
        OutputProxyPort<RTT::extras::ReadOnlyPointer<T2> > &base(*this);
        return port->connectTo(inputPort.getPortInterface());
    }
    
    bool connectTo(InputProxyPort<RTT::extras::ReadOnlyPointer<T> > &inputPort)
    {
        return port->connectTo(inputPort.getPortInterface());
    }
    
    bool disconnect()
    {
        port->disconnect();
        return true;
    }

    bool disconnect(InputProxyPort<T> &inputPort)
    {
        port->disconnect(inputPort.getPortInterface());
        return true;
    }

    ~OutputProxyPort()
    {
        deleteReader();
    }
};
