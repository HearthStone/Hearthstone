/***
 * Demonstrike Core
 */

#include "platform.h"
#include "ReferenceCount.h"

namespace G3D {

ReferenceCountedObject::ReferenceCountedObject() : 
	ReferenceCountedObject_refCount(0), 
	ReferenceCountedObject_weakPointer(0) {

	ASSERT(isValidHeapPointer(this));
}

void ReferenceCountedObject::ReferenceCountedObject_zeroWeakPointers() {
	// Tell all of my weak pointers that I'm gone.
	
	_WeakPtrLinkedList* node = ReferenceCountedObject_weakPointer;

	while (node != NULL) {
		// Notify the weak pointer that it is going away
		node->weakPtr->objectCollected();

		// Free the node and advance
		_WeakPtrLinkedList* tmp = node;
		node = node->next;
		delete tmp;
	}
}

ReferenceCountedObject::~ReferenceCountedObject() {}


ReferenceCountedObject::ReferenceCountedObject(const ReferenceCountedObject& notUsed) : 
	ReferenceCountedObject_refCount(0),
	ReferenceCountedObject_weakPointer(0) {
	(void)notUsed;
	ASSERT(isValidHeapPointer(this));
}

ReferenceCountedObject& ReferenceCountedObject::operator=(const ReferenceCountedObject& other) {
	(void)other;
	// Nothing changes when I am assigned; the reference count on
	// both objects is the same (although my super-class probably
	// changes).
	return *this;
}

} // G3D
