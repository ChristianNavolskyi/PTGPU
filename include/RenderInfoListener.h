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


