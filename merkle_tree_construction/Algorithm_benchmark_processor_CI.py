import csv
import numpy as np
import os
import re
import scipy.stats
import sys
import time

# Given a regular expression, list the files that match it, and ask for user input
def selectFile(regex, subdirs = False):
	files = []
	if subdirs:
		for (dirpath, dirnames, filenames) in os.walk('.'):
			for file in filenames:
				path = os.path.join(dirpath, file)
				if path[:2] == '.\\': path = path[2:]
				if bool(re.match(regex, path)):
					files.append(path)
	else:
		for file in os.listdir(os.curdir):
			if os.path.isfile(file) and bool(re.match(regex, file)):
				files.append(file)
	
	print()
	if len(files) == 0:
		print(f'No files were found that match "{regex}"')
		print()
		return ''
	elif len(files) == 1:
		return files[0]

	print('List of files:')
	for i, file in enumerate(files):
		print(f'  File {i + 1}  -  {file}')
	print()

	selection = None
	while selection is None:
		try:
			i = int(input(f'Please select a file (1 to {len(files)}): '))
		except KeyboardInterrupt:
			sys.exit()
		except:
			pass
		if i > 0 and i <= len(files):
			selection = files[i - 1]
	print()
	return selection


# Given a regular expression, list the directories that match it, and ask for user input
def selectDir(regex, subdirs = False):
	dirs = []
	if subdirs:
		for (dirpath, dirnames, filenames) in os.walk('.'):
			if dirpath[:2] == '.\\': dirpath = dirpath[2:]
			if bool(re.match(regex, dirpath)):
				dirs.append(dirpath)
	else:
		for obj in os.listdir(os.curdir):
			if os.path.isdir(obj) and bool(re.match(regex, obj)):
				dirs.append(obj)

	print()
	if len(dirs) == 0:
		print(f'No directories were found that match "{regex}"')
		print()
		return ''

	print('List of directories:')
	for i, directory in enumerate(dirs):
		print(f'  Directory {i + 1}  -  {directory}')
	print()

	selection = None
	while selection is None:
		try:
			i = int(input(f'Please select a directory (1 to {len(dirs)}): '))
		except KeyboardInterrupt:
			sys.exit()
		except:
			pass
		if i > 0 and i <= len(dirs):
			selection = dirs[i - 1]
	print()
	return selection

# List the files with a regular expression
def listFiles(regex, directory = ''):
	path = os.path.join(os.curdir, directory)
	return [os.path.join(path, file) for file in os.listdir(path) if os.path.isfile(os.path.join(path, file)) and bool(re.match(regex, file))]

# Change the color of text in the terminal
# Leaving the forground or background blank will reset the color to its default
# Providing a message will return the colored message (reset to default afterwards)
# If it's not working for you, be sure to call os.system('cls') before modifying colors
# Usage:
# - print(color('black', 'white', 'Inverted') + ' Regular')
# - print(color('black', 'white') + 'Inverted' + color() + ' Regular')
def color(foreground = '', background = '', message = ''):
	fg = {
		'red': '1;31',
		'green': '1;32',
		'yellow': '1;33',
		'blue': '1;34',
		'purple': '1;35',
		'cyan': '1;36',
		'white': '1;37',
		'black': '0;30',
		'gray': '1;30'
	}
	bg = {
		'red': ';41m',
		'green': ';42m',
		'yellow': ';43m',
		'blue': ';44m',
		'purple': ';45m',
		'cyan': ';46m',
		'white': ';47m',
		'black': ';48m'
	}
	if foreground not in fg or background not in bg: return '\033[0m' # Reset
	color = f'\033[0m\033[{ fg[foreground.lower()] }{ bg[background.lower()] }'

	if message == '': return color
	else: return f'{ color }{ str(message) }\033[0m'

# Return the +- value after a set of data
def mean_confidence_interval(data, confidence=0.95):
	a = 1.0 * np.array(data)
	n = len(a)
	m, se = np.mean(a), scipy.stats.sem(a)
	h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
	return h

def row_count(filename):
	with open(filename) as in_file:
		return sum(1 for _ in in_file)

# Input format: ../bitcoin-X/src
def processPathToVersion(path):
	match = re.match(r'.*bitcoin-([^\/]+)', path)
	return 'v' + match.group(1)

inputFileName = selectFile('Algorithm_benchmark_[0-9]+.csv')
outputFilePath = 'Algorithm_benchmark_CI_output.csv'
outputFile = open(outputFilePath, 'w', newline='')

header = ''
header += 'Version,'
header += 'Construct leaves (ms),'
header += 'CI Construct leaves (ms),'
header += 'Form tree (ms),'
header += 'CI Form tree (ms),'
header += 'Generate proof (ms),'
header += 'CI Generate proof (ms),'
header += 'Verify proof (ms),'
header += 'CI Verify proof (ms),'
header += 'Number of files,'
header += 'CI Number of files,'
outputFile.write(header + '\n')

print(f'Processing "{inputFileName}"...')

last_line_number = row_count(inputFileName)
readerFile = open(inputFileName, 'r')
reader = csv.reader(x.replace('\0', '') for x in readerFile)

temp_header = next(reader)
rowCounter = 0

dynamic_list_bitcoin_version = ''
dynamic_list_1 = []
dynamic_list_2 = []
dynamic_list_3 = []
dynamic_list_4 = []
dynamic_list_5 = []


for row in reader:
	bitcoin_version = row[0]

	if dynamic_list_bitcoin_version == '':
		dynamic_list_bitcoin_version = bitcoin_version

	isLastLine = (last_line_number == reader.line_num)

	if bitcoin_version == dynamic_list_bitcoin_version and not isLastLine:
		dynamic_list_1.append(float(row[1]))
		dynamic_list_2.append(float(row[2]))
		dynamic_list_3.append(float(row[3]))
		dynamic_list_4.append(float(row[4]))
		dynamic_list_5.append(float(row[5]))

	else:
		print(f'Processing version "{bitcoin_version}"')
		mean1 = sum(dynamic_list_1) / len(dynamic_list_1)
		ci1 = mean_confidence_interval(dynamic_list_1)
		mean2 = sum(dynamic_list_2) / len(dynamic_list_2)
		ci2 = mean_confidence_interval(dynamic_list_2)
		mean3 = sum(dynamic_list_3) / len(dynamic_list_3)
		ci3 = mean_confidence_interval(dynamic_list_3)
		mean4 = sum(dynamic_list_4) / len(dynamic_list_4)
		ci4 = mean_confidence_interval(dynamic_list_4)
		mean5 = sum(dynamic_list_5) / len(dynamic_list_5)
		ci5 = mean_confidence_interval(dynamic_list_5)

		line = ''
		line += processPathToVersion(dynamic_list_bitcoin_version) + ','
		line += str(mean1) + ','
		line += str(ci1) + ','
		line += str(mean2) + ','
		line += str(ci2) + ','
		line += str(mean3) + ','
		line += str(ci3) + ','
		line += str(mean4) + ','
		line += str(ci4) + ','
		line += str(mean5) + ','
		line += str(ci5) + ','
		outputFile.write(line + '\n')

		dynamic_list_bitcoin_version = bitcoin_version
		dynamic_list_1 = []
		dynamic_list_2 = []
		dynamic_list_3 = []
		dynamic_list_4 = []
		dynamic_list_5 = []

readerFile.close()

print(f'\nAverage column outputs saved to "{outputFilePath}".')
print('Goodbye.')
sys.exit()
