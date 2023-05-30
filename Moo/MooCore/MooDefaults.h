#pragma once

#define MOO_NO_COPY(Class)\
Class(const Class&) = delete;\
Class& operator=(const Class&) = delete;\
Class(Class&&) = default;\
Class& operator=(Class&&) = default

#define MOO_DELETE_DEFAULTS(Class)\
Class(const Class&) = delete;\
Class& operator=(const Class&) = delete;\
Class(Class&&) = delete;\
Class& operator=(Class&&) = delete

#define MOO_DEFAULTS(Class)\
Class(const Class&) = default;\
Class& operator=(const Class&) = default;\
Class(Class&&) = default;\
Class& operator=(Class&&) = default
