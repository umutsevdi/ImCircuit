#include <algorithm>
#include <cstring>
#include <sstream>
#include <utility>
#include "common.h"
#include "core.h"

namespace ic {

Scene::Scene(const std::string& _name, const std::string& _author,
        const std::string& _description, int _version) :
        version { _version }, component_context { std::nullopt }, frame_s{0},
    _last_node {
            Node { 0, Node::Type::GATE },
            Node { 0, Node::Type::COMPONENT },
            Node { 0, Node::Type::INPUT },
            Node { 0, Node::Type::OUTPUT },
        },
        _last_rel { 0 }
{
    set_name(_name);
    set_author(_author);
    set_description(_description);
}

Scene::Scene(ComponentContext ctx, const std::string& _name,
        const std::string& _author, const std::string& _description, int _version) :
        version { _version }, component_context { ctx }, frame_s{0},
        _last_node {
            Node { 0, Node::Type::GATE },
            Node { 0, Node::Type::COMPONENT },
            Node { 0, Node::Type::INPUT },
            Node { 0, Node::Type::OUTPUT },
        },
        _last_rel { 0 }
{
    set_name(_name);
    set_author(_author);
    set_description(_description);
}

Scene::Scene(Scene&& other)
{
    if (this != &other) {
        _move_from(std::move(other));
    }
}

Scene& Scene::operator=(Scene&& other)
{
    if (this != &other) {
        _move_from(std::move(other));
    }
    return *this;
}

void Scene::clone(const Scene& other)
{
    memcpy(_name.data(), other._name.data(), _name.size());
    memcpy(_description.data(), other._description.data(), _description.size());
    memcpy(_author.data(), other._author.data(), _author.size());
    version = other.version;
    for (const Scene& dep : other.dependencies()) {
        Scene s {};
        s.clone(dep);
        add_dependency(std::move(s));
    }

    frame_s           = other.frame_s;
    _gates            = other._gates;
    _components       = other._components;
    _inputs           = other._inputs;
    _outputs          = other._outputs;
    _relations        = other._relations;
    component_context = other.component_context;
    for (size_t i = 0; i < Node::Type::NODE_S; i++) {
        _last_node[i] = other._last_node[i];
    }
    _last_rel = other._last_rel;
    for (auto& gate : _gates) {
        gate.reload(this);
    }
    for (auto& comp : _components) {
        comp.reload(this);
    }
    for (auto& input : _inputs) {
        input.reload(this);
    }
    for (auto& output : _outputs) {
        output.reload(this);
    }
    if (component_context.has_value()) {
        component_context->reload(this);
    }
}

const std::array<char, 128>& Scene::name(void) const { return _name; }
const std::array<char, 60>& Scene::author(void) const { return _author; }
const std::array<char, 512>& Scene::description(void) const
{
    return _description;
}
Error Scene::set_name(const std::string& name)
{
    if (name.size() < _name.size()) {
        std::string old { _name.data() };
        undo.push([this, old]() {
            std::strncpy(_name.data(), old.c_str(),
                std::min(_name.size() - 1, old.size()));
        });
        std::strncpy(_name.data(), name.c_str(),
            std::min(_name.size() - 1, name.size()));
        return Error::OK;
    }
    return ERROR(Error::INVALID_STRING);
}
Error Scene::set_author(const std::string& author)
{
    if (author.size() < _author.size()) {
        std::string old { _author.data() };
        undo.push([this, old]() {
            std::strncpy(_author.data(), old.c_str(),
                std::min(_author.size() - 1, old.size()));
        });
        std::strncpy(_author.data(), author.c_str(),
            std::min(_author.size() - 1, author.size()));
        return Error::OK;
    }
    return ERROR(Error::INVALID_STRING);
}
Error Scene::set_description(const std::string& description)
{
    if (description.size() < _description.size()) {
        std::string old { _description.data() };
        undo.push([this, old]() {
            std::strncpy(_description.data(), old.c_str(),
                std::min(_description.size() - 1, old.size()));
        });
        std::strncpy(_description.data(), description.c_str(),
            std::min(_description.size() - 1, _description.size()));
        return Error::OK;
    }
    return ERROR(Error::INVALID_STRING);
}

void Scene::_move_from(Scene&& other)
{
    _name             = std::move(other._name);
    _description      = std::move(other._description);
    _author           = std::move(other._author);
    version           = other.version;
    _dependencies     = std::move(other._dependencies);
    frame_s           = other.frame_s;
    _gates            = std::move(other._gates);
    _components       = std::move(other._components);
    _inputs           = std::move(other._inputs);
    _outputs          = std::move(other._outputs);
    _relations        = std::move(other._relations);
    component_context = std::move(other.component_context);
    for (size_t i = 0; i < Node::Type::NODE_S; i++) {
        _last_node[i] = other._last_node[i];
    }
    _last_rel = other._last_rel;

    for (auto& gate : _gates) {
        gate.reload(this);
    }
    for (auto& comp : _components) {
        comp.reload(this);
    }
    for (auto& input : _inputs) {
        input.reload(this);
    }
    for (auto& output : _outputs) {
        output.reload(this);
    }
    for (auto& dep : _dependencies) {
        dep._parent = this;
    }
    if (component_context.has_value()) {
        component_context->reload(this);
    }
}

Error Scene::duplicate_node(Node& id)
{
    // Add node to the back regardless to make it simpler
    L_INFO("Duplicate %s@%d", to_str(id.type), id.index);
    switch (id.type) {
    case Node::GATE: {
        auto node = get_node<Gate>(id);
        if (node == nullptr) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        auto g { *node };
        g.reload(this);
        if (_last_node[id.type].index == _gates.size()) {
            _last_node[id.type].index++;
        }
        _gates.push_back(g);
        id.index = _gates.size() - 1;
        break;
    }
    case Node::COMPONENT: {
        auto node = get_node<Component>(id);
        if (node == nullptr) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        auto g { *node };
        g.reload(this);
        if (_last_node[id.type].index == _components.size()) {
            _last_node[id.type].index++;
        }
        _components.push_back(g);
        id.index = _components.size() - 1;
        break;
    }
    case Node::INPUT: {
        auto node = get_node<Input>(id);
        if (node == nullptr) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        auto g { *node };
        g.reload(this);
        if (_last_node[id.type].index == _inputs.size()) {
            _last_node[id.type].index++;
        }
        _inputs.push_back(g);
        id.index = _inputs.size() - 1;
        break;
    }
    case Node::OUTPUT: {
        auto node = get_node<Output>(id);
        if (node == nullptr) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        auto g { *node };
        g.reload(this);
        if (_last_node[id.type].index == _outputs.size()) {
            _last_node[id.type].index++;
        }
        _outputs.push_back(g);
        id.index = _outputs.size() - 1;
        break;
    }
    default: return ERROR(Error::INVALID_NODE);
    }
    return Error::OK;
}

Error Scene::remove_node(Node id)
{
    auto node = get_base(id);
    if (node == nullptr) {
        return ERROR(Error::NODE_NOT_FOUND);
    }
    node->clean();
    node->set_null();
    if (_last_node[id.type].index >= id.index) {
        _last_node[id.type].index = id.index;
    }
    L_DEBUG("%s@%d was invalidated. Last node for %s was updated to %d",
        to_str<Node::Type>(id.type), id.index, to_str<Node::Type>(id.type),
        _last_node[id.type].index);
    L_INFO(
        "Removed %s@%d from the scene.", to_str<Node::Type>(id.type), id.index);
    return Error::OK;
}

Ref<const Rel> Scene::get_rel(relid idx) const
{
    if (idx == 0 || idx > _last_rel) {
        return nullptr;
    }
    auto n = _relations.find(idx);
    return n != _relations.end() ? &n->second : nullptr;
}

Ref<Rel> Scene::get_rel(relid idx)
{
    if (idx == 0 || idx > _last_rel) {
        return nullptr;
    }
    auto n = _relations.find(idx);
    return n != _relations.end() ? &n->second : nullptr;
}

Error Scene::connect_with_id(
    relid id, Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    if (id == 0) {
        _last_rel++;
        id = _last_rel;
    }

    if (from_node.type == Node::Type::OUTPUT
        || from_node.type == Node::Type::COMPONENT_OUTPUT) {
        return ERROR(Error::INVALID_FROM_TYPE);
    } else if (from_node.type == Node::Type::COMPONENT
        && from_sock >= dependencies()[get_node<Component>(from_node)->dep_idx]
                .component_context->outputs.size()) {
        return ERROR(Error::INVALID_NODEID);
    }
    if (!component_context.has_value()
        && (to_node.type == Node::Type::COMPONENT_OUTPUT
            || from_node.type == Node::Type::COMPONENT_INPUT)) {
        return ERROR(Error::NOT_A_COMPONENT);
    }

    switch (to_node.type) {
    case Node::Type::GATE: {
        auto gate = get_node<Gate>(to_node);
        if (gate == nullptr) {
            return ERROR(Error::INVALID_TO_TYPE);
        } else if (gate->inputs[to_sock] != 0) {
            return ERROR(Error::ALREADY_CONNECTED);
        }
        gate->inputs[to_sock] = id;
        break;
    }
    case Node::Type::COMPONENT: {
        auto comp = get_node<Component>(to_node);
        if (comp == nullptr) {
            return ERROR(Error::INVALID_TO_TYPE);
        } else if (comp->inputs[to_sock] != 0) {
            return ERROR(Error::ALREADY_CONNECTED);
        }
        comp->inputs[to_sock] = id;
        break;
    }
    case Node::Type::OUTPUT: {
        auto out = get_node<Output>(to_node);
        if (out == nullptr) {
            return ERROR(Error::INVALID_TO_TYPE);
        } else if (out->input != 0) {
            return ERROR(Error::ALREADY_CONNECTED);
        }
        out->input = id;
        break;
    }
    case Node::Type::COMPONENT_OUTPUT: {
        if (component_context->outputs.size() > to_node.index - 1) {
            relid& to_id = component_context->outputs[to_node.index - 1];
            if (to_id != 0) {
                return ERROR(Error::ALREADY_CONNECTED);
            }
            to_id = id;
        }
        break;
    }
    default: return ERROR(Error::INVALID_TO_TYPE);
    }
    switch (from_node.type) {
    case Node::Type::GATE: {
        auto from = get_node<Gate>(from_node);
        if (from == nullptr) {
            return ERROR(Error::INVALID_FROM_TYPE);
        }
        from->output.push_back(id);
        break;
    }
    case Node::Type::COMPONENT: {
        auto from = get_node<Component>(from_node);
        if (from == nullptr) {
            return ERROR(Error::INVALID_FROM_TYPE);
        }
        from->outputs[from_sock].push_back(id);
        break;
    }
    case Node::Type::INPUT: {
        auto from = get_node<Input>(from_node);
        if (from == nullptr) {
            return ERROR(Error::INVALID_FROM_TYPE);
        }
        from->output.push_back(id);
        break;
    }
    case Node::Type::COMPONENT_INPUT: /* Component input is not handled here. */
        ic_assert(component_context.has_value());
        if (component_context->inputs.size() > from_node.index - 1) {
            std::vector<relid>& from_list
                = component_context->inputs[from_node.index - 1];
            for (auto _id : from_list) {
                if (_id == id) {
                    return ERROR(Error::ALREADY_CONNECTED);
                }
            }
            from_list.push_back(id);
        }
        break;
    default: return ERROR(Error::INVALID_TO_TYPE);
    }
    _relations.emplace(id, Rel { id, from_node, to_node, from_sock, to_sock });
    if (from_node.type != Node::COMPONENT_INPUT) {
        get_base(from_node)->on_signal();
    } else {
        component_context->run(0, 0);
    }
    L_INFO("Created connection between %s@%d and %s@%d with the id %d.",
        to_str<Node::Type>(from_node.type), from_node.index,
        to_str<Node::Type>(to_node.type), to_node.index, id);
    undo.push([this, id]() { disconnect(id); });
    return OK;
}

relid Scene::connect(
    Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    return connect_with_id(0, to_node, to_sock, from_node, from_sock)
        ? 0
        : _last_rel;
}

Error Scene::disconnect(relid id)
{
    if (id == 0 || id > _last_rel) {
        return ERROR(Error::INVALID_RELID);
    }
    auto remove_fn = [id](relid i) { return i == id; };
    auto r         = _relations.find(id);
    if (r == _relations.end()) {
        return ERROR(Error::REL_NOT_FOUND);
    }

    switch (r->second.from_node.type) {
    case Node::Type::GATE: {
        auto node = get_node<Gate>(r->second.from_node);
        if (node->is_null()) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        node->output.erase(std::remove_if(
            node->output.begin(), node->output.end(), remove_fn));
        break;
    }
    case Node::Type::COMPONENT: {
        auto& v = get_node<Component>(r->second.from_node)
                      ->outputs[r->second.from_sock];
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case Node::Type::INPUT: {
        auto node = get_node<Input>(r->second.from_node);
        if (node->is_null()) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        node->output.erase(std::remove_if(
            node->output.begin(), node->output.end(), remove_fn));
        break;
    }
    case Node::Type::COMPONENT_INPUT: {
        ic_assert(component_context.has_value());
        if (component_context->inputs.size() > r->second.from_node.index - 1) {
            auto& v = component_context->inputs[r->second.from_node.index - 1];
            v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: ic_assert(r->second.from_node.type == Node::Type::OUTPUT); break;
    }

    switch (r->second.to_node.type) {
    case Node::Type::GATE: {
        auto g = get_node<Gate>(r->second.to_node);
        if (g->is_null()) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        g->inputs[r->second.to_sock] = 0;
        g->on_signal();
        break;
    }
    case Node::Type::COMPONENT: {
        auto c = get_node<Component>(r->second.to_node);
        if (c->is_null()) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        c->inputs[r->second.to_sock] = 0;
        c->on_signal();
        break;
    }
    case Node::Type::OUTPUT: {
        auto o = get_node<Output>(r->second.to_node);
        if (o->is_null()) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        o->input = 0;
        o->on_signal();
        break;
    }
    case Node::Type::COMPONENT_OUTPUT: {
        ic_assert(component_context.has_value());
        if (component_context->outputs.size() > r->second.to_node.index - 1) {
            component_context->outputs[r->second.to_node.index - 1] = 0;
            component_context->run();
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: ic_assert(r->second.from_node.type == Node::Type::INPUT); break;
    }
    L_INFO("Disconnected %s@%d from %s@%d.",
        to_str<Node::Type>(r->second.from_node.type), r->second.from_node.index,
        to_str<Node::Type>(r->second.to_node.type), r->second.to_node.index);
    undo.push([this, r](void) {
        Error _ = this->connect_with_id(r->first, r->second.to_node,
            r->second.to_sock, r->second.from_node, r->second.from_sock);
    });
    _relations.erase(id);
    return OK;
}

void Scene::signal(relid id, State value)
{
    ic_assert(id != 0);
    auto r = get_rel(id);
    ic_assert(r != nullptr);
    if (r->value != value || r->value == DISABLED) {
        r->value = value;
        L_DEBUG("%s:rel@%-2d %s@%d:%d sent %s to %s@%d:%d",
            _parent != nullptr ? name().data() : "root", id,
            to_str<Node::Type>(r->from_node.type), r->from_node.index,
            r->from_sock, to_str<State>(r->value),
            to_str<Node::Type>(r->to_node.type), r->to_node.index, r->to_sock);
        if (r->to_node.type != Node::Type::COMPONENT_OUTPUT) {
            auto n = get_base(r->to_node);
            ic_assert(n != nullptr);
            n->on_signal();
        } else {
            component_context->set_value(r->to_node.index, r->value);
        }
    }
}

Ref<BaseNode> Scene::get_base(Node id)
{
    switch (id.type) {
    case Node::Type::GATE: return get_node<Gate>(id)->base();
    case Node::Type::COMPONENT: return get_node<Component>(id)->base();
    case Node::Type::INPUT: return get_node<Input>(id)->base();
    case Node::Type::OUTPUT: return get_node<Output>(id)->base();
    default: break;
    }
    return nullptr;
}

std::string Scene::to_dependency() const
{
    std::stringstream dep_str {};
    std::string_view str_author { author().data() };
    if (str_author.empty()) {
        dep_str << "local/";
    } else {
        dep_str << str_author << '/';
    }
    dep_str << std::string_view { name().data() } << '/'
            << std::to_string(version);
    return dep_str.str();
}

void Scene::run(float delta)
{
    uint32_t frame_pre = frame_s * 10;
    frame_s += delta;
    uint32_t frame = frame_s * 10;
    if (frame != frame_pre) {
        for (auto& in : _inputs) {
            if (!in.is_null() && in.is_timer()) {
                if (frame % in.freq() == 0) {
                    in.set(frame / 10 % 2 == 0);
                }
            }
        }
    }
}

Error Scene::add_dependency(const std::string& name)
{
    // TODO Implement
    return Error::OK;
}
void Scene::add_dependency(Scene&& scene)
{
    _dependencies.emplace_back(std::move(scene));
    _dependencies.back()._parent = this;
    undo.push([this]() { _dependencies.pop_back(); });
}

void Scene::remove_dependency(size_t idx)
{
    size_t i = 0;
    while (i < _components.size()) {
        if (_components[i].dep_idx == idx) {
            remove_node(i);
        } else {
            i++;
        }
    }
    _dependencies.erase(_dependencies.begin() + idx);
}

} // namespace ic
