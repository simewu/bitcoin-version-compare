import os
import re
import sys
import difflib
import filecmp
import itertools
#import charmap

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
def listFiles(regex, directory = '', subdirs = False):
	#path = os.path.join(os.curdir, directory)
	#return [os.path.join(path, file) for file in os.listdir(path) if os.path.isfile(os.path.join(path, file)) and bool(re.match(regex, file))]
	files = []
	if subdirs:
		for (dirpath, dirnames, filenames) in os.walk(directory):
			for file in filenames:
				path = os.path.join(dirpath, file)
				if path[:2] == '.\\': path = path[2:]
				if bool(re.match(regex, path)):
					files.append(path)
	else:
		for file in os.listdir(os.curdir):
			if os.path.isfile(file) and bool(re.match(regex, file)):
				files.append(file)
	return files

def listDirectories(regex, subdirs = False, reverse = True):
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
		return []

	if reverse:
		dirs.reverse()

	print('List of directories:')
	for i, directory in enumerate(dirs):
		print(f'  Directory {i + 1}  -  {directory}')
	print()
	return dirs


# Taken from https://gist.github.com/amorton/1024636
def compareDirectories(expected_dir, actual_dir, header=None):
	"""Compares two directories and diffs any different files.
	
	The file list of both directories must match and the files must match.
	Sub directories are not considered as I don't need to in my case.
	
	The diff will not show that a file was deleted from one side. It shows
	that all the lines were removed.
	
	:raises: AssertionError if there are differences. Error will 
	contain the diff as it's message.  
	
	:param expected_dir: directory of expected files
	
	:param actual_dir: directory of actual files
	
	:param header: Optional string header to prepend to the diff.
	"""
	
	if os.path.exists(expected_dir) and os.path.exists(actual_dir):
		dir_diff = filecmp.dircmp(expected_dir, actual_dir)
		diff_files = list(itertools.chain(dir_diff.diff_files, 
			dir_diff.left_only, dir_diff.right_only))
	else:
		try:
			diff_files = os.listdir(actual_dir)
		except (OSError):
			diff_files = os.listdir(expected_dir)

	if not diff_files:
		return
		
	sb = []
	if header:
		sb.append(header)
		
	def safe_read_lines(path):
		if not os.path.exists(path):
			return []
		f = open(path, encoding = 'utf8')
		try:
			return f.readlines()
		finally:
			f.close()
			
	for diff_file in diff_files:
		expected_file = os.path.join(expected_dir, diff_file)
		actual_file = os.path.join(actual_dir, diff_file)
		
		if os.path.isdir(expected_file) or os.path.isdir(actual_file):
			#print(expected_file)
			#print(actual_file)
			continue
			raise RuntimeError("Unexpected sub dir")
			
		diff = difflib.unified_diff(
		#diff = difflib.context_diff(
			safe_read_lines(expected_file),
			safe_read_lines(actual_file),
			fromfile=expected_file, 
			tofile=actual_file)
		sb.extend(diff)

	return "".join(sb)
	#self.fail(msg="".join(sb))


def getDirectoryStats(directory, prevVersionDirectory):
	files = listFiles('.', directory, True)
	filesSize = 0
	for file in files:
		filesSize += os.path.getsize(file)

	codeFiles = listFiles('(.*\.cpp|.*\.h|.*\.py|.*\.c|.*\.sh)', directory, True)
	codefilesSize = 0
	for file in codeFiles:
		codefilesSize += os.path.getsize(file)

	extensionsDict = {}
	for file in files:
		extension = os.path.splitext(file)[1]
		if extension not in extensionsDict:
			extensionsDict[extension] = 1
		else:
			extensionsDict[extension] += 1
	extensionsDict = dict(sorted(extensionsDict.items(), key=lambda item: item[1], reverse=True))

	extensions = ''
	for key in extensionsDict:
		if len(extensions):
			extensions += ', '
		extensions += key + ' (' + str(extensionsDict[key]) + ')'



	a = compareDirectories(prevVersionDirectory, directory)
	print(a)


	print(directory + ':')
	print(len(files))
	return {
		'Directory': directory,
		'Num files': len(files),
		'Size files (B)': filesSize,
		'Num code files (cpp, py, c, h, sh)': len(codeFiles),
		'Size code files (B)': codefilesSize,
		'Extenension histogram': extensions,
		'Prev Version Comparison': 'blag'
	}


dirs = listDirectories(r'.*', False, False)

outputFileName = 'logDirectoryOutput.csv'
outputFile = open(outputFileName, 'w')
headerMade = False

prevVersionDirectory = ''

for directory in dirs:
	data = getDirectoryStats(directory, prevVersionDirectory)
	if not headerMade:
		header = ''
		for key in data:
			header += '"' + key + '",'
		outputFile.write(header + '\n')
		headerMade = True

	line = ''
	for key in data:
		line += '"' + str(data[key]) + '",'
	outputFile.write(line + '\n')

	prevVersionDirectory = directory


print('Successfully wrote to "' + outputFileName + '".')