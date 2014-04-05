lib:
	mkdir -p build
	g++ -shared -fPIC -o build/libimgui.so.0.1 -Iinclude src/imgui.cpp src/imguiRenderGL3.cpp

clean:
	rm -rf build

install:
	cp build/libimgui.so.0.1 /usr/local/lib/libimgui.so.0.1
	rm -rf /usr/local/lib/libimgui.so
	ln -s /usr/local/lib/libimgui.so.0.1 /usr/local/lib/libimgui.so
	mkdir -p /usr/local/include/imgui
	cp include/* /usr/local/include/imgui/

sample:
	$(MAKE) -C samples
