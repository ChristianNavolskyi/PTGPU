//
// Created by Melina Christian Navolskyi on 08.01.20.
//

#pragma once


// TODO make interface
class RenderInfoListener
{

public:
	RenderInfoListener() = default;

	virtual ~RenderInfoListener() = default;

	virtual void notify() = 0;

	virtual void notifySizeChanged(int newWidth, int newHeight) = 0;
};


