/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of NMODL distributed under the terms of the GNU
 * Lesser General Public License. See top-level LICENSE file for details.
 *************************************************************************/

///
/// THIS FILE IS GENERATED AT BUILD TIME AND SHALL NOT BE EDITED.
///

#pragma once

/**
 *
 * \dir
 * \brief Auto generated visitors
 *
 * \file
 * \brief \copybrief nmodl::visitor::CheckParentVisitor
 */

#include "visitors/ast_visitor.hpp"

namespace nmodl {
namespace visitor {
namespace test {

/**
 * @ingroup visitor_classes
 * @{
 */

/**
 * \class CheckParentVisitor
 * \brief %Visitor for checking parents of ast nodes
 *
 * The visitor goes down the tree (parent -> children) marking down in
 * parent who is the parent of the node he is visiting.
 * Once check_parent(ast::Ast* node) verified that the current node has
 * the correct parent, we set the current node as parent and go down
 * the tree. Once all the children were checked we set the parent of the
 * current node as parent (it was checked before) and return.
 */
class CheckParentVisitor : public AstVisitor {
    private:
        /**
        * \brief Keeps track of the parents while going down the tree
        */
        ast::Ast* parent = nullptr;
        /**
        * \brief Flag to activate the parent check on the root node
        *
        * This flag tells to the visitor to check (or not check) if the
        * root node, from which we start the visit, is the root node and
        * thus, it should have nullptr parent
        */
        bool is_root_with_null_parent = false;
    public:

        /**
        * \brief Standard constructor
        *
        * If is_root_with_null_parent is set to true, also the initial
        * node is checked to be sure that is really the root (parent = nullptr)
        */
        CheckParentVisitor(const bool is_root_with_null_parent = true) {}

        /**
        * \brief A small wrapper to have a nicer call in parser.cpp
        */
        int check_ast(ast::Ast* node);

        /**
        * \brief Check the parent, throw an error if not
        */
        void check_parent(ast::Ast* node) const;

        {% for node in nodes %}
        /**
        * \brief Go through the tree while checking the parents
        */
        void visit_{{ node.class_name|snake_case }}(ast::{{ node.class_name }}* node) override;
        {% endfor %}
};

/** @} */  // end of visitor_classes

}  // namespace test
}  // namespace visitor
}  // namespace nmodl