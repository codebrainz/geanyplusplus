import Face
import re
import string
import sys

__all__ = [
	"CodeGen",
	"Constant",
	"Enumeration",
	"Enumerator",
	"Function",
	"Interface",
	"Lexer",
	"Parameter",
	"Property",
	"Style",
	"Visitor",
]

def camel_to_under(name):
	s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
	return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

class Constant:
	def __init__(self, name, value):
		self.name = name
		if isinstance(value, str):
			self.value = int(value, 0)
		else:
			self.value = value
		self.comment = None
	def accept(self, v, *args, **kwargs):
		return v.visit_Constant(self, *args, **kwargs)

class Enumerator(Constant):
	def __init__(self, name, value):
		super().__init__(name, value)
	def accept(self, v, *args, **kwargs):
		return v.visit_Enumerator(self, *args, **kwargs)

class Enumeration:
	def __init__(self, name, prefixes=[]):
		self.name = name
		self.prefixes = prefixes
		self.enumerators = []
		self.comment = None
	def accept(self, v, *args, **kwargs):
		return v.visit_Enumeration(self, *args, **kwargs)

class Style(Constant):
	def __init__(self, name, value):
		super().__init__(name, value)
	def accept(self, v, *args, **kwargs):
		return v.visit_Style(self, *args, **kwargs)

class Lexer(Constant):
	def __init__(self, name, value, ident, prefixes=[]):
		super().__init__(name, value)
		self.ident = ident
		self.prefixes = prefixes
		self.styles = []
	def accept(self, v, *args, **kwargs):
		return v.visit_Lexer(self, *args, **kwargs)

class Parameter:
	def __init__(self, type, name, value):
		self.type = type
		self.name = camel_to_under(name)
		self.value = value
		self.comment = None
	def accept(self, v, *args, **kwargs):
		return v.visit_Parameter(self, *args, **kwargs)

class Function(Constant):
	def __init__(self, name, value, type, params=[]):
		super().__init__(name, value)
		self.type = type
		self.parameters = params
	def accept(self, v, *args, **kwargs):
		return v.visit_Function(self, *args, **kwargs)

class Property:
	def __init__(self, name, getter=None, setter=None):
		self.name = name
		self.getter = getter
		self.setter = setter
		self.comment = None
	def accept(self, v, *args, **kwargs):
		return v.visit_Property(self, *args, **kwargs)

class Event(Function):
	def __init__(self, name, value, type, params=[]):
		super().__init__(name, value, type, params)
	def accept(self, v, *args, **kwargs):
		return v.visit_Event(self, *args, **kwargs)

class Interface:

	def __init__(self, fn):
		self.filename = fn
		self.constants = []
		self.properties = []
		self.functions = []
		self.events = []
		self.enumerations = []
		self.lexers = []
		self.symbols = {}

		self.parse_()
		self.post_process_()

	def accept(self, v, *args, **kwargs):
		return v.visit_Interface(self, *args, **kwargs)

	def add_sym_(self, sym, val):
		if sym in self.symbols:
			raise Exception("duplicate symbol '%s' encountered" % sym)
		self.symbols[sym] = val

	def parse_(self):
		iface = Face.Face()
		iface.ReadFromFile(self.filename)
		prop_table = {}

		for name in iface.order:
			feature = iface.features[name]

			if feature["FeatureType"] == "fun":
				param1 = Parameter(feature["Param1Type"], feature["Param1Name"], feature["Param1Value"])
				param2 = Parameter(feature["Param2Type"], feature["Param2Name"], feature["Param2Value"])
				uname = camel_to_under(name)
				fun = Function(uname, feature["Value"], feature["ReturnType"], [param1, param2])
				fun.comment = feature["Comment"]
				self.add_sym_(uname, fun)
				self.functions.append(fun)

			elif feature["FeatureType"] == "get":
				param1 = Parameter(feature["Param1Type"], feature["Param1Name"], feature["Param1Value"])
				param2 = Parameter(feature["Param2Type"], feature["Param2Name"], feature["Param2Value"])
				uname = camel_to_under(name)
				fun = Function(uname, feature["Value"], feature["ReturnType"], [param1, param2])
				fun.comment = feature["Comment"]
				if uname not in prop_table:
					prop = Property(uname)
					self.add_sym_(uname, prop)
					prop_table[uname] = prop
					self.properties.append(prop)
				prop = prop_table[uname]
				prop.getter = fun

			elif feature["FeatureType"] == "set":
				param1 = Parameter(feature["Param1Type"], feature["Param1Name"], feature["Param1Value"])
				param2 = Parameter(feature["Param2Type"], feature["Param2Name"], feature["Param2Value"])
				uname = camel_to_under(name)
				fun = Function(uname, feature["Value"], feature["ReturnType"], [param1, param2])
				fun.comment = feature["Comment"]
				if uname not in prop_table:
					prop = Property(uname)
					self.add_sym_(uname, prop)
					prop_table[uname] = prop
					self.properties.append(prop)
				prop = prop_table[uname]
				prop.setter = fun

			elif feature["FeatureType"] == "enu":
				if name in self.symbols:
					enu = self.symbols[name]
					enu.prefixes = list(set(enu.prefixes + feature["Prefixes"]))
					enu.prefixes.extend(feature["Prefixes"])
					enu.comment.extend(feature["Comment"])
				else:
					enu = Enumeration(name, feature["Prefixes"])
					enu.comment = feature["Comment"]
					self.add_sym_(name, enu)
					self.enumerations.append(enu)

			elif feature["FeatureType"] == "evt":
				uname = camel_to_under(name)
				params = [ Parameter(p[0], p[1], p[2]) for p in feature["Params"] ]
				evt = Event(uname, feature["Value"], feature["ReturnType"], params)
				evt.comment = feature["Comment"]
				self.add_sym_(uname, evt)
				self.events.append(evt)

		# collect constants belonging to enums
		for enum in self.enumerations:
			for name in iface.order:
				feature = iface.features[name]
				if feature["FeatureType"] == "val":
					if any(name.startswith(x) for x in enum.prefixes):
						etor = Enumerator(name, feature["Value"])
						#etor.comment = feature["Comment"]
						self.add_sym_(name, etor)
						enum.enumerators.append(etor)

		# collect lexers
		for name in iface.order:
			feature = iface.features[name]
			if feature["FeatureType"] == "lex":
				lex_ident = feature["Value"]
				lex_value = self.symbols[lex_ident].value
				lex = Lexer(name, lex_value, lex_ident, feature["Prefixes"])
				lex.comment = feature["Comment"]
				#print('lexer "%s" prefixes = %s' % (lex.name, lex.prefixes))
				self.add_sym_(name, lex)
				self.lexers.append(lex)

		# collect constants belonging to lexers
		for lexer in self.lexers:
			for name in iface.order:
				feature = iface.features[name]
				if feature["FeatureType"] == "val":
					if any(name.startswith(x) for x in lexer.prefixes):
						style = Style(name, feature["Value"])
						#style.comment = feature["Comment"]
						#print("Name '%s' for lexer '%s' with prefixes = %s" % (name, lexer.name, lexer.prefixes))
						#self.add_sym_(name, style)
						self.symbols[name] = style
						lexer.styles.append(style)

		# get any constants that aren't enumerators or lexer styles
		for name in iface.order:
			feature = iface.features[name]
			if feature["FeatureType"] == "val":
				if name not in self.symbols:
					const = Constant(name, feature["Value"])
					#const.comment = feature["Comment"]
					self.add_sym_(name, const)
					self.constants.append(const)

	def post_process_(self):

		def fix_ident(name):
			if name == '8859_15':
				name = 'ISO_' + name
			elif name == 'NULL':
				name = 'NULL_LEXER'
			return name

		def strip_enum_prefix(enum, name):
			# first handle special cases that don't follow the iface conventions
			if enum.name == "ModificationFlags":
				if name.startswith("SC_"):
					name = name[3:]
				return name
			elif enum.name == "FoldAction":
				if name.startswith("SC_FOLDACTION_"):
					name = name[14:]
				return name
			elif enum.name == "FontQuality":
				if name.startswith("SC_PHASES_"):
					return name[3:]
			# normal prefix stripping
			for prefix in enum.prefixes:
				if name.startswith(prefix):
					return name[len(prefix):]
			return name

		def strip_lexer_prefix(lexer, name):
			for prefix in lexer.prefixes:
				if name.startswith(prefix):
					name = name[len(prefix):]
					break
			return name

		# remove prefixes from enumerators
		for enum in self.enumerations:
			for etor in enum.enumerators:
				etor.name = fix_ident(strip_enum_prefix(enum, etor.name))

		# remove prefixes from lexer styles
		for lexer in self.lexers:
			for style in lexer.styles:
				style.name = fix_ident(strip_lexer_prefix(lexer, style.name))

class Visitor:
	def visit_Constant(self, n, *args, **kwargs):
		pass
	def visit_Enumerator(self, n, *args, **kwargs):
		pass
	def visit_Enumeration(self, n, *args, **kwargs):
		pass
	def visit_Style(self, n, *args, **kwargs):
		pass
	def visit_Lexer(self, n, *args, **kwargs):
		pass
	def visit_Parameter(self, n, *args, **kwargs):
		pass
	def visit_Function(self, n, *args, **kwargs):
		pass
	def visit_Property(self, n, *args, **kwargs):
		pass
	def visit_Event(self, n, *args, **kwargs):
		pass
	def visit_Interface(self, n, *args, **kwargs):
		pass

class CodeGen:
	def __init__(self, out, ind_lvl=0, ind_tp='\n'):
		self.out = out
		self.ind_lvl = ind_lvl
		self.ind_tp = ind_tp
		self.ind_str = self.ind_tp * self.ind_lvl
	def update_indent_(self):
		self.ind_str = self.ind_tp * self.ind_lvl
	def indent(self):
		self.ind_lvl += 1
		self.update_indent_()
	def unindent(self):
		self.ind_lvl -= 1
		self.update_indent_()
	def write(self, msg=''):
		self.out.write(msg)
	def writeln(self, msg=''):
		self.write(msg + '\n')
	def iwrite(self, msg=''):
		self.write(self.ind_str + msg)
	def iwriteln(self, msg=''):
		self.iwrite(msg + '\n')
