/*
 * opencog/atomspace/Atom.cc
 *
 * Copyright (C) 2002-2007 Novamente LLC
 * All Rights Reserved
 *
 * Written by Thiago Maia <thiago@vettatech.com>
 *            Andre Senna <senna@vettalabs.com>
 *            Welter Silva <welter@vettalabs.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Atom.h"

#include <set>

#ifndef WIN32
#include <unistd.h>
#endif

#include <opencog/util/platform.h>

#include <opencog/atomspace/AtomSpaceDefinitions.h>
#include <opencog/atomspace/AtomTable.h>
#include <opencog/atomspace/ClassServer.h>
#include <opencog/atomspace/Defaults.h>
#include <opencog/atomspace/Link.h>
#include <opencog/atomspace/StatisticsMonitor.h>
#include <opencog/atomspace/TLB.h>
#include <opencog/util/Logger.h>
#include <opencog/util/exceptions.h>
#include <opencog/util/misc.h>

//#define USE_SHARED_DEFAULT_TV

#undef Type

using namespace opencog;

void Atom::init(Type t, const std::vector<Handle>& outg, const TruthValue& tv )
{
    // resets all flags
    flags = 0;
    atomTable = NULL;
    incoming = NULL;
    type = t;

#ifndef PUT_OUTGOING_SET_IN_LINKS
    // need to call the method to handle specific subclass case
    setOutgoingSet(outg);
#endif /* PUT_OUTGOING_SET_IN_LINKS */

#ifdef USE_SHARED_DEFAULT_TV
    truthValue = NULL;
    setTruthValue(tv);
#else
    truthValue = tv.isNullTv() ? TruthValue::DEFAULT_TV().clone() : tv.clone();
#endif
}

Atom::Atom(Type type, const std::vector<Handle>& outgoingVector, const TruthValue& tv )
{
    init(type, outgoingVector, tv);
}

Atom::~Atom() throw (RuntimeException)
{
    // checks if there is still an atom pointing to the one being removed
    if (incoming != NULL) {
        throw RuntimeException(TRACE_INFO, "Attempting to remove atom with non-empty incoming set.");
    }

#ifdef USE_SHARED_DEFAULT_TV
    if (truthValue != &(TruthValue::DEFAULT_TV())) {
        delete truthValue;
    }
#else
    delete truthValue;
#endif
}

bool Atom::isReal() const
{
    unsigned long value = (unsigned long) this;
    bool real = (value >= NUMBER_OF_CLASSES);
    //if(!real){
    //fprintf(stdout, "Atom - Type: %d, Pointe converted: (%p, %d)\n", type, this, (long)this);
    //fflush(stdout);
    //}
    return real;
}

const AttentionValue& Atom::getAttentionValue() const
{
    return attentionValue;
}

const TruthValue& Atom::getTruthValue() const
{
    return *truthValue;
}

void Atom::setTruthValue(const TruthValue& tv)
{
#ifdef USE_SHARED_DEFAULT_TV
    if (truthValue != NULL && &tv != truthValue && truthValue != &(TruthValue::DEFAULT_TV())) {
        delete truthValue;
    }
    truthValue = (TruthValue*) & (TruthValue::DEFAULT_TV());
    if (!tv.isNullTv() && (&tv != &(TruthValue::DEFAULT_TV()))) {
        truthValue = tv.clone();
    }
#else
    if (truthValue != NULL && &tv != truthValue) {
        delete truthValue;
    }
#if 1
    // just like it was before
    truthValue = tv.clone();
#else
    if (!tv.isNullTv()) {
        truthValue = tv.clone();
    } else {
        truthValue = TruthValue::DEFAULT_TV().clone();
    }
#endif
#endif
}

//float Atom::getImportance() {

//    return ShortFloatOps::getValue(&importance);
//}

void Atom::setAttentionValue(const AttentionValue& new_av) throw (RuntimeException)
{
    if (new_av == attentionValue) return;

    int oldBin = -1;
    if (atomTable != NULL) {
        // gets current bin
        oldBin = ImportanceIndex::importanceBin(attentionValue.getSTI());
    }

    attentionValue = new_av;

    if (atomTable != NULL) {
        // gets new bin
        int newBin = ImportanceIndex::importanceBin(attentionValue.getSTI());

        // if the atom importance has changed its bin,
        // updates the importance index
        if (oldBin != newBin) {
            atomTable->updateImportanceIndex(this, oldBin);
            StatisticsMonitor::getInstance()->atomChangeImportanceBin(type, oldBin, newBin);
        }
    }
}

#ifndef PUT_OUTGOING_SET_IN_LINKS
void Atom::setOutgoingSet(const std::vector<Handle>& outgoingVector)  throw (RuntimeException)
{
    //printf("Atom::setOutgoingSet\n");
    if (atomTable != NULL) {
        throw RuntimeException(TRACE_INFO, "Cannot change the OutgoingSet of an atom already inserted into an AtomTable\n");
    }
#ifdef USE_STD_VECTOR_FOR_OUTGOING
#ifdef PEFORM_INVALID_HANDLE_CHECKS
    // Make sure that garbage is not being passed in.
    // We'd like to perform a test for valid values here, but it seems
    // the NMXmlParser code intentionally adds Handle::UNDEFINED to link nodes,
    // which it hopefully repairs later on ...
    for (int i = 0; i < outgoingVector.size(); i++) {
        if (TLB::isInvalidHandle(outgoingVector[i])) {
            throw RuntimeException(TRACE_INFO, "setOutgoingSet was passed invalid handles\n");
        }
    }
#endif
    outgoing = outgoingVector;
    // if the link is unordered, it will be normalized by sorting the elements in the outgoing list.
    if (ClassServer::isAssignableFrom(UNORDERED_LINK, type)) {
        std::sort(outgoing.begin(), outgoing.end(), CoreUtils::HandleComparison());
    }
#else
    if (outgoing) {
        free(outgoing);
        outgoing = NULL;
    }
    arity = outgoingVector.size();
    if (arity > 0) {
        outgoing = (Handle*) malloc(sizeof(Handle) * arity);
        for (int i = 0; i < arity; i++) {
#ifdef PEFORM_INVALID_HANDLE_CHECKS
            // We'd like to perform a test for valid values here, but it seems
            // the NMXmlParser code intentionally adds Handle::UNDEFINED to link nodes,
            // which it hopefully repairs later on ...
            if (TLB::isInvalidHandle(outgoingVector[i])) {
                throw RuntimeException(TRACE_INFO, "setOutgoingSet was passed invalid handles\n");
            }
#endif
            outgoing[i] = outgoingVector[i];
        }
    }
    if (ClassServer::isAssignableFrom(UNORDERED_LINK, type)) {
        qsort(outgoing, arity, sizeof(Handle), CoreUtils::handleCompare);
    }
#endif
}

void Atom::addOutgoingAtom(Handle h)
{
#ifdef PEFORM_INVALID_HANDLE_CHECKS
    // We'd like to perform a test for valid values here, but it seems
    // the NMXmlParser code intentionally adds Handle::UNDEFINED to link nodes,
    // which it hopefully repairs later on ...
    if (TLB::isInvalidHandle(h))
        throw RuntimeException(TRACE_INFO, "addOutgoingAtom was passed invalid handles\n");
#endif
#ifdef USE_STD_VECTOR_FOR_OUTGOING
    outgoing.push_back(h);
#else
    outgoing = (Handle*) realloc(outgoing, (arity + 1) * sizeof(Handle));
    outgoing[arity++] = h;
#endif
}

Atom * Atom::getOutgoingAtom(int position) const throw (RuntimeException)
{
    // checks for a valid position
    if ((position < getArity()) && (position >= 0)) {
        return TLB::getAtom(outgoing[position]);
    } else {
        throw RuntimeException(TRACE_INFO, "invalid outgoing set index %d", position);
    }
}
#endif /* PUT_OUTGOING_SET_IN_LINKS */

void Atom::addIncomingHandle(Handle handle)
{

    // creates a new entry with handle
    HandleEntry* entry = new HandleEntry(handle);
    // entry is placed in the first position of the incoming set
    entry->next = incoming;
    incoming = entry;
}

void Atom::removeIncomingHandle(Handle handle) throw (RuntimeException)
{

    //printf("Entering Atom::removeIncomingHandle(): handle:\n%s\nincoming:\n%s\n", TLB::getAtom(handle)->toShortString().c_str(), incoming->toString().c_str());
    HandleEntry* current = incoming;
    // checks if incoming set is empty
    if (incoming == NULL) {
        throw RuntimeException(TRACE_INFO, "unable to extract incoming element from empty set");
    }

    // checks if the handle to be removed is the first one
    if (incoming->handle == handle) {
        incoming = incoming->next;
        current->next = NULL;
        delete current;
    } else {
        if (current->next == NULL) {
            throw RuntimeException(TRACE_INFO, "unable to extract incoming element");
        }
        // scans the list looking for the desired handle
        while (current->next->handle != handle) {
            current = current->next;
            if (current->next == NULL) {
                throw RuntimeException(TRACE_INFO, "unable to extract incoming element");
            }
        }
        // deletes entry when the handle is found
        HandleEntry* foundit = current->next;
        current->next = foundit->next;
        foundit->next = NULL;
        delete foundit;
    }
    //printf("Exiting Atom::removeIncomingHandle(): incoming:\n%s\n", incoming->toString().c_str());
}

void Atom::merge(Atom* other) throw (InconsistenceException)
{
    if (*this != *other)
        throw InconsistenceException(TRACE_INFO,
             "Different atoms cannot be merged");

    // Merges the incoming set and updates the outgoingSets
    Handle thisHandle = TLB::getHandle(this);
    Handle otherHandle = TLB::getHandle(other);

    HandleEntry *inc = other->getIncomingSet();
    while (inc != NULL) {
        addIncomingHandle(inc->handle);

        // For links, merege the outgoing sets.
        Atom *incAtom = TLB::getAtom(inc->handle);
        Link *link = dynamic_cast<Link *>(incAtom);
        if (link) {
            std::vector<Handle> outgoingSet = link->getOutgoingSet();
            for (int i = 0; i < link->getArity(); i++) {
                if (outgoingSet[i] == otherHandle) {
                    outgoingSet[i] = thisHandle;
                }
            }
            // Although we have direct access to outgoing here, we need to
            // call setOutgoingSet anyway, since special handling may be
            // need by subclasses (e.g, sorting of the outgoing, if it's
            // an unordered link)
            link->setOutgoingSet(outgoingSet);
        }
        inc = inc->next;
    }


    //setImportance((getImportance() + other->getImportance()) / 2);
    //setHeat((getHeat() + other->getHeat()) / 2);

#ifdef USE_SHARED_DEFAULT_TV
    // TruthValue::merge() method always return a new TV object
    TruthValue* mergedTv = truthValue->merge(other->getTruthValue());
    if (truthValue != &(TruthValue::DEFAULT_TV())) {
        delete(truthValue);
    }
    if (mergedTv == &(TruthValue::DEFAULT_TV())) {
        delete(mergedTv);
        truthValue = (TruthValue*) & (TruthValue::DEFAULT_TV());
    } else {
        truthValue = mergedTv;
    }
#else
    TruthValue* mergedTv = truthValue->merge(other->getTruthValue()); // always return a new TV object
    delete(truthValue);
    truthValue = mergedTv;
#endif

    //cprintf(DEBUG, ">> This atom's truth values after merge: %f %f %f\n", truthValue->getMean(), truthValue->getConfidence(), truthValue->getCount());
}

bool Atom::getFlag(int flag) const
{
    return (flags & flag) != 0;
}

void Atom::setFlag(int flag, bool value)
{
    if (value) {
        flags |= flag;
    } else {
        flags &= ~(flag);
    }
}

void Atom::unsetRemovalFlag(void)
{
    flags &= ~MARKED_FOR_REMOVAL;
}

void Atom::markForRemoval(void)
{
    flags |= MARKED_FOR_REMOVAL;
}

void Atom::setAtomTable(AtomTable *tb)
{
    atomTable = tb;
}

AtomTable *Atom::getAtomTable() const
{
    return(atomTable);
}

HandleEntry *Atom::getNeighbors(bool fanin, bool fanout, Type desiredLinkType, bool subClasses) const
{

    HandleEntry *answer = NULL;
    Handle me = TLB::getHandle(this);

    for (HandleEntry *h = getIncomingSet(); h != NULL; h = h ->next) {
        Link *link = dynamic_cast<Link*>(TLB::getAtom(h->handle));
        Type linkType = link->getType();
        //printf("linkType = %d desiredLinkType = %d\n", linkType, desiredLinkType);
        if ((linkType == desiredLinkType) || (subClasses && ClassServer::isAssignableFrom(desiredLinkType, linkType))) {
            int linkArity = link->getArity();
            for (int i = 0; i < linkArity; i++) {
                Handle handle = link->getOutgoingSet()[i];
                if (handle == me) continue;
                if (!fanout && link->isSource(me)) continue;
                if (!fanin && link->isTarget(me)) continue;
                HandleEntry *n = new HandleEntry(handle);
                n->next = answer;
                answer = n;
            }
        }
    }

    return answer;
}
