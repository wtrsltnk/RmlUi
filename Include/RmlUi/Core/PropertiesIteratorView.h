/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef RMLUICOREPROPERTIESITERATORVIEW_H
#define RMLUICOREPROPERTIESITERATORVIEW_H

#include "Types.h"

namespace Rml {
namespace Core {

class PropertiesIterator;

/**
	Provides an iterator for properties defined in the element's style or definition.
	Construct it through the desired Element.
	Warning: Modifying the underlying style invalidates the iterator.

	Usage:
		for(auto it = element.IterateLocalProperties(); !it.AtEnd(); ++it) { ... }

	Note: Not an std-style iterator. Implemented as a wrapper over the internal
	iterator to avoid exposing internal headers to the user.

	@author Michael R. P. Ragazzon
 */

class RMLUICORE_API PropertiesIteratorView {
public:
	PropertiesIteratorView(std::unique_ptr<PropertiesIterator> ptr);
	PropertiesIteratorView(PropertiesIteratorView&& other);
	PropertiesIteratorView& operator=(PropertiesIteratorView&& other);
	PropertiesIteratorView(const PropertiesIteratorView& other) = delete;
	PropertiesIteratorView& operator=(const PropertiesIteratorView&) = delete;
	~PropertiesIteratorView();

	PropertiesIteratorView& operator++();

	// Returns true when all properties have been iterated over.
	// @warning The getters and operator++ can only be called if the iterator is not at the end.
	bool AtEnd() const;

	PropertyId GetId() const;
	const String& GetName() const;
	const Property& GetProperty() const;

	// Returns the list of pseudo classes which defines the current property, possibly empty.
	// @todo Currently returns an empty list.
	const PseudoClassList& GetPseudoClassList() const;

private:
	std::unique_ptr<PropertiesIterator> ptr;
};

}
}

#endif