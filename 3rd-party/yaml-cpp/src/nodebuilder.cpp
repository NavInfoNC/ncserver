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

#include <assert.h>
#include <cassert>

#include "nodebuilder.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"

namespace YAML {
struct Mark;

NodeBuilder::NodeBuilder()
    : m_pMemory(new detail::memory_holder),
      m_pRoot(nullptr),
      m_stack{},
      m_anchors{},
      m_keys{},
      m_mapDepth(0) {
  m_anchors.push_back(nullptr);  // since the anchors start at 1
}

NodeBuilder::~NodeBuilder() {}

Node NodeBuilder::Root() {
  if (!m_pRoot)
    return Node();

  return Node(*m_pRoot, m_pMemory);
}

void NodeBuilder::OnDocumentStart(const Mark&) {}

void NodeBuilder::OnDocumentEnd() {}

void NodeBuilder::OnNull(const Mark& mark, anchor_t anchor) {
  detail::node& node = Push(mark, anchor);
  node.set_null();
  Pop();
}

void NodeBuilder::OnAlias(const Mark& /* mark */, anchor_t anchor) {
  detail::node& node = *m_anchors[anchor];
  Push(node);
  Pop();
}

void NodeBuilder::OnScalar(const Mark& mark, const std::string& tag,
                           anchor_t anchor, const std::string& value) {
  detail::node& node = Push(mark, anchor);
  node.set_scalar(value);
  node.set_tag(tag);
  Pop();
}

void NodeBuilder::OnSequenceStart(const Mark& mark, const std::string& tag,
                                  anchor_t anchor, EmitterStyle::value style) {
  detail::node& node = Push(mark, anchor);
  node.set_tag(tag);
  node.set_type(NodeType::Sequence);
  node.set_style(style);
}

void NodeBuilder::OnSequenceEnd() { Pop(); }

void NodeBuilder::OnMapStart(const Mark& mark, const std::string& tag,
                             anchor_t anchor, EmitterStyle::value style) {
  detail::node& node = Push(mark, anchor);
  node.set_type(NodeType::Map);
  node.set_tag(tag);
  node.set_style(style);
  m_mapDepth++;
}

void NodeBuilder::OnMapEnd() {
  assert(m_mapDepth > 0);
  m_mapDepth--;
  Pop();
}

detail::node& NodeBuilder::Push(const Mark& mark, anchor_t anchor) {
  detail::node& node = m_pMemory->create_node();
  node.set_mark(mark);
  RegisterAnchor(anchor, node);
  Push(node);
  return node;
}

void NodeBuilder::Push(detail::node& node) {
  const bool needsKey =
      (!m_stack.empty() && m_stack.back()->type() == NodeType::Map &&
       m_keys.size() < m_mapDepth);

  m_stack.push_back(&node);
  if (needsKey)
    m_keys.push_back(PushedKey(&node, false));
}

void NodeBuilder::Pop() {
  assert(!m_stack.empty());
  if (m_stack.size() == 1) {
    m_pRoot = m_stack[0];
    m_stack.pop_back();
    return;
  }

  detail::node& node = *m_stack.back();
  m_stack.pop_back();

  detail::node& collection = *m_stack.back();

  if (collection.type() == NodeType::Sequence) {
    collection.push_back(node, m_pMemory);
  } else if (collection.type() == NodeType::Map) {
    assert(!m_keys.empty());
    PushedKey& key = m_keys.back();
    if (key.second) {
      collection.insert(*key.first, node, m_pMemory);
      m_keys.pop_back();
    } else {
      key.second = true;
    }
  } else {
    assert(false);
    m_stack.clear();
  }
}

void NodeBuilder::RegisterAnchor(anchor_t anchor, detail::node& node) {
  if (anchor) {
    assert(anchor == m_anchors.size());
    m_anchors.push_back(&node);
  }
}
}  // namespace YAML
