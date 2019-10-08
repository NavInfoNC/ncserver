/*
Copyright (c) 2008-2015 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "nodeevents.h"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/mark.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/detail/node_iterator.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"

namespace YAML {
void NodeEvents::AliasManager::RegisterReference(const detail::node& node) {
  m_anchorByIdentity.insert(std::make_pair(node.ref(), _CreateNewAnchor()));
}

anchor_t NodeEvents::AliasManager::LookupAnchor(
    const detail::node& node) const {
  AnchorByIdentity::const_iterator it = m_anchorByIdentity.find(node.ref());
  if (it == m_anchorByIdentity.end())
    return 0;
  return it->second;
}

NodeEvents::NodeEvents(const Node& node)
    : m_pMemory(node.m_pMemory), m_root(node.m_pNode), m_refCount{} {
  if (m_root)
    Setup(*m_root);
}

void NodeEvents::Setup(const detail::node& node) {
  int& refCount = m_refCount[node.ref()];
  refCount++;
  if (refCount > 1)
    return;

  if (node.type() == NodeType::Sequence) {
    for (detail::const_node_iterator it = node.begin(); it != node.end(); ++it)
      Setup(**it);
  } else if (node.type() == NodeType::Map) {
    for (detail::const_node_iterator it = node.begin(); it != node.end();
         ++it) {
      Setup(*it->first);
      Setup(*it->second);
    }
  }
}

void NodeEvents::Emit(EventHandler& handler) {
  AliasManager am;

  handler.OnDocumentStart(Mark());
  if (m_root)
    Emit(*m_root, handler, am);
  handler.OnDocumentEnd();
}

void NodeEvents::Emit(const detail::node& node, EventHandler& handler,
                      AliasManager& am) const {
  anchor_t anchor = NullAnchor;
  if (IsAliased(node)) {
    anchor = am.LookupAnchor(node);
    if (anchor) {
      handler.OnAlias(Mark(), anchor);
      return;
    }

    am.RegisterReference(node);
    anchor = am.LookupAnchor(node);
  }

  switch (node.type()) {
    case NodeType::Undefined:
      break;
    case NodeType::Null:
      handler.OnNull(Mark(), anchor);
      break;
    case NodeType::Scalar:
      handler.OnScalar(Mark(), node.tag(), anchor, node.scalar());
      break;
    case NodeType::Sequence:
      handler.OnSequenceStart(Mark(), node.tag(), anchor, node.style());
      for (detail::const_node_iterator it = node.begin(); it != node.end();
           ++it)
        Emit(**it, handler, am);
      handler.OnSequenceEnd();
      break;
    case NodeType::Map:
      handler.OnMapStart(Mark(), node.tag(), anchor, node.style());
      for (detail::const_node_iterator it = node.begin(); it != node.end();
           ++it) {
        Emit(*it->first, handler, am);
        Emit(*it->second, handler, am);
      }
      handler.OnMapEnd();
      break;
  }
}

bool NodeEvents::IsAliased(const detail::node& node) const {
  RefCount::const_iterator it = m_refCount.find(node.ref());
  return it != m_refCount.end() && it->second > 1;
}
}  // namespace YAML
