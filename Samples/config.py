##
#This script builds the ASPS_config file the main program will use to 
#add custom informations to the program, such as the gravity force.
#As the ASPS_config file is program oriented and not easily to read,
#the current script opens a window allowing the user to modify the config file option
#the easy way
##


import os
from Tkinter import *

def main(_path):

	#If all the program essential informations are not set in the interactive window,
	#a default dictionary will fill the missing elements
	g_defaultConfig =\
	{\
		"Particle mesh" 		: {"type":	"P",	"value":	"sphere.aso"	},
		"Emitter mesh" 			: {"type":	"F",	"value":	"sphere.aso"	},
		"Force field mesh" 		: {"type":	"F",	"value":	"sphere.aso"	},
		"Portal mesh" 			: {"type":	"F",	"value":	"sphere.aso"	},
		"Bouncer mesh"			: {"type":	"F",	"value":	"sphere.aso"	},
		"Emitter texture"		: {"type":	"F",	"value":	""				},
		"Force field texture"	: {"type":	"F",	"value":	""				},
		"Portal texture"		: {"type":	"F",	"value":	""				},
		"Bouncer texture"		: {"type":	"F",	"value":	""				},
		"Particle texture"		: {"type":	"P",	"value":	"BrickSmall.jpg"}, 
		"Particle ramp color" 	: {"type":	"P",	"value":	"ramp.bmp"		},
		"Particle number"		: {"type":	"P",	"value":	"1000"			},
		"Gravity"				: {"type":	"E",	"value":	"9.8"			},
		"Atmosphere thickness"	: {"type":	"E",	"value":	"0.0"			},
		"Fields initial number" : {"type":	"F",	"value":	"5"				},
		"Environment map"		: {"type":	"S",	"value":	""				}\
	}

	g_config = dict(g_defaultConfig)
	#Put userFile informations in a buffer

	#Opens the ASPS_config file and update g_config
	if os.path.exists(_path):
		_f = open(_path, "r")
		_line = _f.readline() 
		while _line:
			if not _line.startswith("//"):
				_line = _line.split("\n")[0]
				t, n, v = _line.split("==>")
				g_config[n] = {'type':t, 'value': v}
			_line = _f.readline()
		_f.close()

	##
	#Build the interactive window with g_config elements
	##

	master = Tk()
	i = 0

	entries = []
	for n, v in g_config.iteritems():
		Label(master, text=n).grid(row=i)
		e = Entry(master)
		#If no value is specified in g_config for this line option
		#Add the corresponding value in g_defaultConfig
		value = v['value']
		if not value:
			value = g_defaultConfig[n]['value']
		e.insert(10, value)
		e.grid(row=i, column=1)
		entries.append((v['type'], n, e))
		i += 1

	##
	#When the user is done with the configuration update,
	#rewrite ASPS_config with each value of the window form
	##
	def callback():
		f = open(_path, 'w')
		for entry in entries:
			objType = entry[0]
			optName = entry[1]
			value	= entry[2].get()
			#If no value is specified in the interactive window for this line option,
			#add the corresponding value in g_defaultConfig
			if not value:
				value = g_defaultConfig[optName]['value']
			f.write("%s==>%s==>%s\n"% (objType, optName, value) )
		f.close()

		Label(master, text="./ASPS_config.cfg updated.").grid(row=i+1)
		b.grid_remove()

	b = Button(master, text="Done", width=10, command=callback)
	b.grid(row=i, column=0)
	Button(master, text='Quit', command=master.quit).grid(row=i, column=1, sticky=W, pady=4)

	mainloop( )


##
#This part opens a dialog window to determine which config file will be updated. 
#It is the entry point of the script and it will open the option window once the config file is specified
##
from tkFileDialog import askopenfilename

file_path = ''

root = Tk()
def open_file():
	global file_path

	file_path = askopenfilename()

	entry.delete(0, END)
	entry.insert(0, file_path)
	return file_path

def process_file(file_path):
	if not file_path.endswith("config.cfg"):
		print "Wrong path %s.\n Looking for file name 'config.cfg'"%file_path
		return
	root.quit()
	main(file_path)

root.title('Choose config file')
root.geometry("598x120+250+100")

mf = Frame(root)
mf.pack()


f1 = Frame(mf, width=600, height=250)
f1.pack(fill=X)
f2 = Frame(mf, width=600, height=250)
f2.pack()

file_path = StringVar


Label(f1,text="Select the config file you want to update (looking for <...>/config.cfg)").grid(row=0, column=0, sticky='e')
entry = Entry(f1, width=50, textvariable=file_path)
entry.grid(row=1,column=0,padx=2,pady=2,sticky='we',columnspan=25)
Button(f1, text="...", command=open_file).grid(row=1, column=27, sticky='ew', padx=8, pady=4)
Button(f2, text="Update config file", width=32, command=lambda: process_file(file_path)).grid(sticky='ew', padx=10, pady=10)


root.mainloop()




'''
##
#This script builds the ASPS_config file the main program will use to 
#add custom informations to the program, such as the gravity force.
#As the ASPS_config file is program oriented and not easily to read,
#the current script opens a window allowing the user to modify the config file option
#the easy way
##


import os
from Tkinter import *

FILE_NAME = "./config.cfg"

#If all the program essential informations are not set in the interactive window,
#a default dictionary will fill the missing elements
g_defaultConfig =\
{\
	"Particle mesh" 		: {"type":	"P",	"value":	"sphere.aso"	},
	"Emitter mesh" 			: {"type":	"F",	"value":	"sphere.aso"	},
	"Force field mesh" 		: {"type":	"F",	"value":	"sphere.aso"	},
	"Portal mesh" 			: {"type":	"F",	"value":	"sphere.aso"	},
	"Bouncer mesh"			: {"type":	"F",	"value":	"sphere.aso"	},
	"Emitter texture"		: {"type":	"F",	"value":	""				},
	"Force field texture"	: {"type":	"F",	"value":	""				},
	"Portal texture"		: {"type":	"F",	"value":	""				},
	"Bouncer texture"		: {"type":	"F",	"value":	""				},
	"Particle texture"		: {"type":	"P",	"value":	"BrickSmall.jpg"}, 
	"Particle ramp color" 	: {"type":	"P",	"value":	"ramp.bmp"		}, 
	"Gravity"				: {"type":	"E",	"value":	"9.8"			},
	"Atmosphere thickness"	: {"type":	"E",	"value":	"0.0"			}\
}

g_config = dict(g_defaultConfig)
#Put userFile informations in a buffer
_path = FILE_NAME

#Opens the ASPS_config file and update g_config
if os.path.exists(_path):
	_f = open(_path, "r")
	_line = _f.readline() 
	while _line:
		if not _line.startswith("//"):
			_line = _line.split("\n")[0]
			t, n, v = _line.split("==>")
			g_config[n] = {'type':t, 'value': v}
		_line = _f.readline()
	_f.close()

##
#Build the interactive window with g_config elements
##

master = Tk()
i = 0

entries = []
for n, v in g_config.iteritems():
	Label(master, text=n).grid(row=i)
	e = Entry(master)
	#If no value is specified in g_config for this line option
	#Add the corresponding value in g_defaultConfig
	value = v['value']
	if not value:
		value = g_defaultConfig[n]['value']
	e.insert(10, value)
	e.grid(row=i, column=1)
	entries.append((v['type'], n, e))
	i += 1

##
#When the user is done with the configuration update,
#rewrite ASPS_config with each value of the window form
##
def callback():
	f = open(FILE_NAME, 'w')
	for entry in entries:
		objType = entry[0]
		optName = entry[1]
		value	= entry[2].get()
		#If no value is specified in the interactive window for this line option,
		#add the corresponding value in g_defaultConfig
		if not value:
			value = g_defaultConfig[optName]['value']
		f.write("%s==>%s==>%s\n"% (objType, optName, value) )
	f.close()

	Label(master, text="./ASPS_config.cfg updated.").grid(row=i+1)
	b.grid_remove()

b = Button(master, text="Done", width=10, command=callback)
b.grid(row=i, column=0)
Button(master, text='Quit', command=master.quit).grid(row=i, column=1, sticky=W, pady=4)

mainloop( )'''