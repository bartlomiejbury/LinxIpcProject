#pragma once

#include "GenericSocket.h"
#include "GenericSimpleServer.h"
#include "GenericServer.h"
#include "GenericClient.h"

template<typename IdentifierType>
struct LinxProtocol {
    using Identifier   = IdentifierType;
    using Socket       = GenericSocket<IdentifierType>;
    using SimpleServer = GenericSimpleServer<IdentifierType>;
    using Server       = GenericServer<IdentifierType>;
    using Client       = GenericClient<IdentifierType>;
};
