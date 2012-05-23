package org.opencv.android;

/**
 * Library installation callback interface
 */
public interface InstallCallbackInterface
{
	/**
	 * Installation of library package is approved
	 */
	public void install();
	/**
	 * Installation canceled;
	 */
	public void cancel();
};