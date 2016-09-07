#!/usr/bin/env python3

import io
import optparse
import os
import re
import sciface
import string
import sys

FUNC_BODIES = {
	"bool_int_void": "return send($msg, $p1);",
	"bool_position_position": "return send($msg, $p1, $p2);",
	"bool_void_void": "return send($msg);",
	"colour_int_void": "return Color::from_int(send($msg, $p1));",
	"colour_void_void": "return Color::from_int(send($msg));",
	"int_int_int": "return send($msg, $p1, $p2);",
	"int_int_string": "return send($msg, $p1, reinterpret_cast<intptr_t>($p2.c_str()));",
	"int_int_stringresult": "return send($msg, $p1, $p2);",
	"int_int_void": "return send($msg, $p1);",
	"int_position_bool": "return send($msg, $p1, $p2);",
	"int_position_position": "return send($msg, $p1, $p2);",
	"int_position_void": "return send($msg, $p1);",
	"int_string_stringresult": "return send($msg, reinterpret_cast<uintptr_t>($p1.c_str()), $p2);",
	"int_string_void": "return send($msg, reinterpret_cast<uintptr_t>($p1.c_str()));",
	"int_void_position": "return send($msg, 0, $p2);",
	"int_void_stringresult": "return send($msg, 0, $p2);",
	"int_void_textrange": "return send($msg, 0, reinterpret_cast<intptr_t>(&$p2));",
	"int_void_void": "return send($msg);",
	"position_bool_formatrange": "return send($msg, $p1, reinterpret_cast<intptr_t>(&$p2));",
	"position_int_findtext": "return send($msg, $p1, reinterpret_cast<intptr_t>(&$p2));",
	"position_int_int": "return send($msg, $p1, $p2);",
	"position_int_void": "return send($msg, $p1);",
	"position_position_int": "return send($msg, $p1, $p2);",
	"position_position_void": "return send($msg, $p1);",
	"position_void_void": "return send($msg);",
	"void_bool_colour": "send($msg, $p1, $p2.to_int());",
	"void_bool_int": "send($msg, $p1, $p2);",
	"void_bool_void": "send($msg, $p1);",
	"void_colour_void": "send($msg, $p1.to_int());",
	"void_int_bool": "send($msg, $p1, $p2);",
	"void_int_cells": "send($msg, $p1, reinterpret_cast<intptr_t>($p2));",
	"void_int_colour": "send($msg, $p1, $p2.to_int());",
	"void_int_int": "send($msg, $p1, $p2);",
	"void_int_position": "send($msg, $p1, $p2);",
	"void_int_string": "send($msg, $p1, reinterpret_cast<intptr_t>($p2.c_str()));",
	"void_int_void": "send($msg, $p1);",
	"void_keymod_int": "send($msg, $p1.to_int(), $p2);",
	"void_keymod_void": "send($msg, $p1.to_int());",
	"void_position_int": "send($msg, $p1, $p2);",
	"void_position_position": "send($msg, $p1, $p2);",
	"void_position_string": "send($msg, $p1, reinterpret_cast<intptr_t>($p2.c_str()));",
	"void_position_void": "send($msg, $p1);",
	"void_string_string": "send($msg, reinterpret_cast<uintptr_t>($p1.c_str()), reinterpret_cast<intptr_t>($p2.c_str()));",
	"void_string_void": "send($msg, reinterpret_cast<uintptr_t>($p1.c_str()));",
	"void_void_int": "send($msg, 0, $p2);",
	"void_void_string": "send($msg, 0, reinterpret_cast<intptr_t>($p2.c_str()));",
	"void_void_void": "send($msg);",
}

FUNC_SIGNATURES = {
	"bool_int_void": "bool $name(int $p1)",
	"bool_position_position": "bool $name(int $p1, int $p2)",
	"bool_void_void": "bool $name()",
	"colour_int_void": "Color $name(int $p1)",
	"colour_void_void": "Color $name()",
	"int_int_int": "int $name(int $p1, int $p2)",
	"int_int_string": "int $name(int $p1, const std::string &$p2)",
	"int_int_stringresult": "int $name(int $p1, std::string &$p2)",
	"int_int_void": "int $name(int $p1)",
	"int_position_bool": "int $name(int $p1, bool $p2)",
	"int_position_position": "int $name(int $p1, int $p2)",
	"int_position_void": "int $name(int $p1)",
	"int_string_stringresult": "int $name(const std::string &$p1, std::string &$p2)",
	"int_string_void": "int $name(const std::string &$p1)",
	"int_void_position": "int $name(int $p2)",
	"int_void_stringresult": "int $name(std::string &$p2)",
	"int_void_textrange": "int $name(Sci_TextRange &$p2)",
	"int_void_void": "int $name()",
	"position_bool_formatrange": "int $name(bool $p1, Sci_RangeToFormat &$p2)",
	"position_int_findtext": "int $name(int $p1, Sci_TextToFind &$p2)",
	"position_int_int": "int $name(int $p1, int $p2)",
	"position_int_void": "int $name(int $p1)",
	"position_position_int": "int $name(int $p1, int $p2)",
	"position_position_void": "int $name(int $p1)",
	"position_void_void": "int $name()",
	"void_bool_colour": "void $name(bool $p1, const Color &$p2)",
	"void_bool_int": "void $name(bool $p1, int $p2)",
	"void_bool_void": "void $name(bool $p1)",
	"void_colour_void": "void $name(const Color &$p1)",
	"void_int_bool": "void $name(int $p1, bool $p2)",
	"void_int_cells": "void $name(int $p1, std::uint8_t *$p2)",
	"void_int_colour": "void $name(int $p1, const Color &$p2)",
	"void_int_int": "void $name(int $p1, int $p2)",
	"void_int_position": "void $name(int $p1, int $p2)",
	"void_int_string": "void $name(int $p1, const std::string &$p2)",
	"void_int_void": "void $name(int $p1)",
	"void_keymod_int": "void $name(const KeyMod &$p1, int $p2)",
	"void_keymod_void": "void $name(const KeyMod &$p1)",
	"void_position_int": "void $name(int $p1, int $p2)",
	"void_position_position": "void $name(int $p1, int $p2)",
	"void_position_string": "void $name(int $p1, const std::string &$p2)",
	"void_position_void": "void $name(int $p1)",
	"void_string_string": "void $name(const std::string &$p1, const std::string &$p2)",
	"void_string_void": "void $name(const std::string &$p1)",
	"void_void_int": "void $name(int $p2)",
	"void_void_string": "void $name(const std::string &$p2)",
	"void_void_void": "void $name()",
}

def format_comment(gen, lines):
	if len(lines) == 0:
		return
	gen.iwriteln('/**')
	for line in lines:
		if line.startswith('#'):
			line = line[1:]
		line = line.strip()
		gen.iwriteln(' * %s' % line)
	gen.iwriteln(' */')

def gen_enums(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for enum in iface.enumerations:
		format_comment(gen, enum.comment)
		gen.iwriteln('enum class %s' % enum.name)
		gen.iwriteln('{')
		gen.indent()
		for etor in enum.enumerators:
			gen.iwriteln('%s = %d,' % (etor.name, etor.value))
		gen.unindent()
		gen.iwriteln('};\n')
	return out.getvalue().rstrip()

def gen_sparse_lexer_names(iface, ind_lvl=0, ind_tp='\t'):
	max_lex = max(lex.value for lex in iface.lexers)
	lexer_names = [ 'nullptr' for x in range(0, max_lex+1) ]
	for lexer in iface.lexers:
		lexer_names[lexer.value] = '"%s"' % lexer.name
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for name in lexer_names:
		gen.iwriteln('%s,' % name)
	return out.getvalue().rstrip()

def gen_lexer_style_defs(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for lexer in iface.lexers:
		if len(lexer.styles) > 0:
			max_style = max(style.value for style in lexer.styles)
		else:
			max_style = 0
		style_names = [ 'nullptr' for x in range(0, max_style+1) ]
		for style in lexer.styles:
			style_names[style.value] = '"%s"' % style.name.lower()
		gen.iwriteln('static const char *%s_styles[%d]' % (lexer.name.lower(), max_style+2))
		gen.iwriteln('{')
		gen.indent()
		for name in style_names:
			gen.iwriteln('%s,' % name)
		gen.iwriteln('nullptr,')
		gen.unindent()
		gen.iwriteln('};')
		gen.iwriteln()
	return out.getvalue().rstrip() + '\n'

def gen_lexer_style_refs(iface, ind_lvl=0, ind_tp='\t'):
	max_lex = max(lex.value for lex in iface.lexers)
	lexer_counts = [ 0 for x in range(0, max_lex+1) ]
	lexer_names = [ 'nullptr' for x in range(0, max_lex+1) ]
	for lexer in iface.lexers:
		if len(lexer.styles) > 0:
			max_style = max(style.value for style in lexer.styles)
		else:
			max_style = 0
		lexer_counts[lexer.value] = max_style+1
		lexer_names[lexer.value] = lexer.name.lower() + '_styles'
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for i in range(0, max_lex+1):
		gen.iwriteln('{ %d, %s },' % (lexer_counts[i], lexer_names[i]))
	return out.getvalue()

def write_if_diff(text, outfn):
	if outfn == '-':
		sys.stdout.write(text)
	elif os.path.exists(outfn):
		orig_text = ''
		with open(outfn) as f:
			orig_text = f.read()
		if text != orig_text:
			with open(outfn, 'w') as f:
				f.write(text)
	else:
		with open(outfn, 'w') as f:
			f.write(text)

def func_typesig(func):
	types = [ func.type ]
	for param in func.parameters:
		if param.type: types.append(param.type)
		else: types.append('void')
	return '_'.join(types)

def format_func_code(code, func):
	d = { "name": func.name, "msg": func.value }
	if len(func.parameters) > 0:
		d["p1"] = func.parameters[0].name
	if len(func.parameters) > 1:
		d["p2"] = func.parameters[1].name
	return string.Template(code).substitute(d)

def gen_function(func, gen):
	typesig = func_typesig(func)
	signature = FUNC_SIGNATURES[typesig]
	body = FUNC_BODIES[typesig]
	format_comment(gen, func.comment)
	gen.iwriteln(format_func_code(signature, func))
	gen.iwriteln('{')
	gen.indent()
	gen.iwriteln(format_func_code(body, func))
	gen.unindent()
	gen.iwriteln('}\n')

def gen_functions(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for func in iface.functions:
		gen_function(func, gen)
	return out.getvalue().rstrip()

def gen_prop_getter(prop, gen):
	if prop.getter is None:
		return
	gen_function(prop.getter, gen)

def gen_prop_setter(prop, gen):
	if prop.setter is None:
		return
	gen_function(prop.setter, gen)

def gen_properties(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for prop in iface.properties:
		gen_prop_getter(prop, gen)
		gen_prop_setter(prop, gen)
	return out.getvalue().rstrip()

def strip_const_prefix(name):
	if name.startswith("SCI_"): # strip SCI_ prefix
		name = name[4:]
	elif name.startswith("SC_"): # strip SC_ prefix
		name = name[3:]
	elif name.startswith("SCEN_"): # SCEN_ -> EN_
		name = name[2:]
	return name

def gen_constant_decls(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	constants = sorted((strip_const_prefix(c.name), c.value) for c in iface.constants)
	for name, value in constants:
		gen.iwriteln('static constexpr int %s = %d;' % (name, value))
	return out.getvalue()

def gen_constant_undefs(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for const in iface.constants:
		gen.iwriteln('#undef %s' % const.name)
	return out.getvalue()

def gen_signals(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for event in iface.events:
		gen.iwriteln('NotificationSignal signal_%s_;' % event.name)
	return out.getvalue().rstrip()

def gen_signal_accessors(iface, ind_lvl=0, ind_tp='\t'):
	out = io.StringIO()
	gen = sciface.CodeGen(out, ind_lvl, ind_tp)
	for event in iface.events:
		gen.iwriteln('NotificationSignal &signal_%s()' % event.name)
		gen.iwriteln('{')
		gen.indent()
		gen.iwriteln('return signal_%s_;' % event.name)
		gen.unindent()
		gen.iwriteln('}\n')
	return out.getvalue().rstrip()

def main(args):

	par = optparse.OptionParser(usage='%prog [-i FILE] [-o FILE] TEMPLATE')

	par.add_option('-i', '--iface', metavar='FILE', dest='iface',
		default=None, help='the Scintilla.iface file to read')
	par.add_option('-o', '--output-file', metavar='FILE', dest='out',
		default='-', help='the output file or - for stdout')

	opts, args = par.parse_args(args[1:])

	if len(args) < 1:
		par.error("not enough arguments, missing input file")
	elif len(args) > 1:
		par.error("too many arguments, expecting single input file")
	template = args[0]

	iface_fn = None
	if opts.iface is None:
		iface_fn = os.path.join('.', 'Scintilla.iface')
		if not os.path.exists(iface_fn):
			iface_fn = os.path.join(os.path.dirname(__file__), 'Scintilla.iface')
			if not os.path.exists(iface_fn):
				par.error("unable to locate 'Scintilla.iface' file, use --iface option to specify")
	else:
		iface_fn = opts.iface

	out_fn = opts.out

	iface = sciface.Interface(iface_fn)

	text = '// This file is auto-generated, do not edit.\n'
	with open(template) as f:
		text += f.read()
	text = text.replace('/*@@enums@@*/', gen_enums(iface, 1))
	text = text.replace('/*@@lexer_names@@*/', gen_sparse_lexer_names(iface, 2))
	text = text.replace('/*@@lexer_style_defs@@*/', gen_lexer_style_defs(iface, 1))
	text = text.replace('/*@@lexer_style_refs@@*/', gen_lexer_style_refs(iface, 2))
	text = text.replace('/*@@methods@@*/', gen_functions(iface, 2))
	text = text.replace('/*@@constant_undefs@@*/', gen_constant_undefs(iface))
	text = text.replace('/*@@constant_decls@@*/', gen_constant_decls(iface, 2))
	text = text.replace('/*@@properties@@*/', gen_properties(iface, 2))
	text = text.replace('/*@@signals@@*/', gen_signals(iface, 2))
	text = text.replace('/*@@signal_accessors@@*/', gen_signal_accessors(iface, 2))

	write_if_diff(text, out_fn)

	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv))
