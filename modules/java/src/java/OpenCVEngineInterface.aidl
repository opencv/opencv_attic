package org.opencv.engine;

interface OpenCVEngineInterface
{
	// Service version
	int getEngineVersion();
	
	// Return path by version. 
	// if package is not installed empty string will be returned
	String getLibPathByVersion(String version);
	
	// Trying to install package from market.
	// Return true if package already installed or installation was successful
	// Return false in case of problems, or if user cancel process
	boolean installVersion(String version);
	
	// Return list of libraries in loading order seporated by ";" symbol
	String getLibraryList(String version);
}