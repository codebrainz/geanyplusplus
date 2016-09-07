#pragma once

#include <gtkmm.h>

extern "C"
{
#include <geanyplugin.h>
#include <Scintilla.h>
#include <SciLexer.h>
#ifndef GTK
#define GTK
#define GEANY_HPP_DEFINED_GTK
#endif
#include <ScintillaWidget.h>
#ifdef GEANY_HPP_DEFINED_GTK
#undef GTK
#endif
}

// prevent namespace pollution
#undef documents
#undef filetypes
