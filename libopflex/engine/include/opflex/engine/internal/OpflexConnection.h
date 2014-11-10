/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file OpflexConnection.h
 * @brief Interface definition file for OpflexConnection
 */
/*
 * Copyright (c) 2014 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string>
#include <stdint.h>
#include <sstream>
#include <list>
#include <utility>

#include <boost/noncopyable.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <uv.h>

#pragma once
#ifndef OPFLEX_ENGINE_OPFLEXCONNECTION_H
#define OPFLEX_ENGINE_OPFLEXCONNECTION_H

namespace opflex {
namespace engine {
namespace internal {

class OpflexPool;
class OpflexHandler;
class HandlerFactory;

/**
 * Maintain the connection state information for a connection to an
 * opflex peer
 */
class OpflexConnection : private boost::noncopyable {
public:

    /**
     * Create a new opflex connection for the given hostname and port
     *
     * @param handlerFactory a factory that can allocate a handler for
     * the connection
     */
    OpflexConnection(HandlerFactory& handlerFactory);
    virtual ~OpflexConnection();

    /**
     * Connect to the remote host.
     */
    virtual void connect();

    /**
     * Disconnect this connection from the remote peer.
     */
    virtual void disconnect();

    /**
     * Get the unique name for this component in the policy domain
     *
     * @returns the string name
     */
    virtual const std::string& getName() = 0;

    /**
     * Get the globally unique name for this policy domain
     *
     * @returns the string domain name
     */
    virtual const std::string& getDomain() = 0;

    /**
     * Check whether the connection is ready to accept requests by
     * calling the opflex handler
     *
     * @return true if the connection is ready
     */
    virtual bool isReady();

    /**
     * Get a human-readable view of the name of the remote peer
     *
     * @return the string name
     */
    virtual const std::string& getRemotePeer() = 0;

    /**
     * Write the given string buffer to the socket.  Ownership of the
     * object passes to the connection
     *
     * @param buf the buffer to write
     */
    virtual void write(const rapidjson::StringBuffer* buf) = 0;

protected:
    /**
     * The handler for the connection
     */
    OpflexHandler* handler;

    friend class OpflexHandler;

    std::stringstream* buffer;
    rapidjson::Document document;

    /**
     * Write an opflex message to the connection using the given
     * string buffer, ownership of which will pass to the connection
     */
    void write(uv_stream_t* stream, const rapidjson::StringBuffer* buf);
    static void write_cb(uv_write_t* req, int status);

    static void read_cb(uv_stream_t* stream, 
                        ssize_t nread, 
                        const uv_buf_t* buf);
    static void alloc_cb(uv_handle_t* handle,
                         size_t suggested_size,
                         uv_buf_t* buf);
    virtual void dispatch();

    typedef std::pair<uv_write_t*, const rapidjson::StringBuffer*> write_t;
    typedef std::list<write_t> write_list_t;
    write_list_t write_list;
    uv_mutex_t write_mutex;
};

/**
 * A factory that will manufacture new handlers for connections
 */
class HandlerFactory {
public:
    /**
     * Allocate a new OpflexHandler for the connection
     *
     * @param the connection associated with the handler
     * @return a newly-allocated handler that will be owned by the
     * connection
     */
    virtual OpflexHandler* newHandler(OpflexConnection* conn) = 0;
};

} /* namespace internal */
} /* namespace engine */
} /* namespace opflex */

#endif /* OPFLEX_ENGINE_OPFLEXCONNECTION_H */