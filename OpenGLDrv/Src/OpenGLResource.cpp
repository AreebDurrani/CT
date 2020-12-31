#include "OpenGLResource.h"

#include "../Inc/OpenGLRenderDevice.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : RenDev(InRenDev),
                                                                                   CacheId(InCacheId),
                                                                                   Revision(-1),
                                                                                   HashIndex(INDEX_NONE),
                                                                                   HashNext(NULL){
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource(){
	RenDev->RemoveResource(this);
}

// FOpenGLShader

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                               Program(GL_NONE){}

FOpenGLShader::~FOpenGLShader(){
	if(Program)
		glDeleteProgram(Program);
}

void FOpenGLShader::Cache(FShaderGLSL* Shader){
	GLuint VertexShader = CompileShader(Shader, GL_VERTEX_SHADER);
	GLuint FragmentShader = CompileShader(Shader, GL_FRAGMENT_SHADER);

	// Set revision even if compilation is unsuccessful to avoid recompiling the invalid shader each time it is set
	Revision = Shader->GetRevision();

	if(!VertexShader || !FragmentShader){
		if(VertexShader)
			glDeleteShader(VertexShader);

		if(FragmentShader)
			glDeleteShader(FragmentShader);

		return;
	}

	GLuint NewProgram = glCreateProgram();

	glAttachShader(NewProgram, VertexShader);
	glAttachShader(NewProgram, FragmentShader);
	glLinkProgram(NewProgram);
	glDetachShader(NewProgram, VertexShader);
	glDetachShader(NewProgram, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	GLint Status;

	glGetProgramiv(NewProgram, GL_LINK_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetProgramInfoLog(NewProgram, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader program linking failed for %s: %s", Shader->GetName(), Buffer);
		glDeleteProgram(NewProgram);
	}else{
		if(Program)
			glDeleteProgram(Program);

		Program = NewProgram;
	}
}

void FOpenGLShader::Bind() const{
	checkSlow(Program);
	glUseProgram(Program);
}

GLuint FOpenGLShader::CompileShader(FShaderGLSL* Shader, GLenum Type){
	GLuint Handle = glCreateShader(Type);
	const TCHAR* ShaderText = NULL;
	const TCHAR* FileExt = NULL;

	if(Type == GL_VERTEX_SHADER){
		ShaderText = Shader->GetVertexShaderText();
		FileExt = VERTEX_SHADER_FILE_EXTENSION;
	}else if(Type == GL_FRAGMENT_SHADER){
		ShaderText = Shader->GetFragmentShaderText();
		FileExt = FRAGMENT_SHADER_FILE_EXTENSION;
	}else{
		appErrorf("Unsupported shader type (%i)", Type);
	}

	glShaderSource(Handle, 1, &ShaderText, NULL);
	glCompileShader(Handle);

	GLint Status;

	glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s%s: %s", Shader->GetName(), FileExt, Buffer);
		glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
}

// FOpenGLIndexBuffer

FOpenGLIndexBuffer::FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                           EBO(GL_NONE),
                                                                                                           IndexSize(0),
                                                                                                           BufferSize(0),
                                                                                                           IsDynamic(InIsDynamic){}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer(){
	Free();
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer){
	if(!IsDynamic)
		Free();

	if(!EBO)
		glCreateBuffers(1, &EBO);

	INT NewBufferSize = IndexBuffer->GetSize();
	void* Data = RenDev->GetScratchBuffer(NewBufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(EBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(EBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(EBO, NewBufferSize, Data, 0);
	}

	IndexSize = IndexBuffer->GetIndexSize();
	Revision = IndexBuffer->GetRevision();
}

void FOpenGLIndexBuffer::Free(){
	if(EBO){
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
}

void FOpenGLIndexBuffer::Bind() const{
	checkSlow(EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}

// FOpenGLVertexStream

FOpenGLVertexStream::FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                             VBO(GL_NONE),
                                                                                                             Stride(0),
                                                                                                             BufferSize(0),
                                                                                                             IsDynamic(InIsDynamic){}

FOpenGLVertexStream::~FOpenGLVertexStream(){
	Free();
}

void FOpenGLVertexStream::Cache(FVertexStream* VertexStream){
	if(!IsDynamic){
		Free();
		IsDynamic = VertexStream->HintDynamic() != 0;
	}

	if(!VBO)
		glCreateBuffers(1, &VBO);

	INT NewBufferSize = VertexStream->GetSize();
	void* Data = NULL;

	VertexStream->GetRawStreamData(&Data, 0);

	if(!Data){
		Data = RenDev->GetScratchBuffer(NewBufferSize);
		VertexStream->GetStreamData(Data);
	}

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(VBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(VBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(VBO, BufferSize, Data, 0);
	}

	Stride = VertexStream->GetStride();
	Revision = VertexStream->GetRevision();
}

void FOpenGLVertexStream::Free(){
	if(VBO){
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
}

void FOpenGLVertexStream::Bind(GLuint BindingIndex) const{
	checkSlow(VBO);
	glBindVertexBuffer(BindingIndex, VBO, 0, Stride);
}

// FOpenGLTexture

FOpenGLTexture::FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                 Width(0),
                                                                                 Height(0),
                                                                                 TextureHandle(GL_NONE),
                                                                                 FBO(GL_NONE),
                                                                                 DepthStencilAttachment(GL_NONE){}

FOpenGLTexture::~FOpenGLTexture(){
	Free();
}

void FOpenGLTexture::Cache(FBaseTexture* BaseTexture){
	Free();

	FTexture* Texture = BaseTexture->GetTextureInterface();
	FRenderTarget* RenderTarget = BaseTexture->GetRenderTargetInterface();

	Width = BaseTexture->GetWidth();
	Height = BaseTexture->GetHeight();

	if(Texture){
		if(Width == 0 || Height == 0){
			static FSolidColorTexture ErrorTexture(FColor(255, 0, 255));

			Texture = &ErrorTexture;
			Width = Texture->GetWidth();
			Height = Texture->GetHeight();
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);

		ETextureFormat SrcFormat = Texture->GetFormat();

		if(IsDXTC(SrcFormat)){
			GLenum GLFormat;

			if(SrcFormat == TEXF_DXT1)
				GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			else if(SrcFormat == TEXF_DXT3)
				GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			else
				GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

			void* Data = Texture->GetRawTextureData(0);

			checkSlow(Data);

			glCompressedTextureImage2DEXT(TextureHandle, GL_TEXTURE_2D, 0, GLFormat, Width, Height, 0, GetBytesPerPixel(SrcFormat, Width * Height), Data);
		}else{
			void* Data = ConvertTextureData(Texture, TEXF_RGBA8, Width, Height, 0);

			glTextureStorage2D(TextureHandle, Texture->GetNumMips(), GL_RGBA8, Width, Height);
			glTextureSubImage2D(TextureHandle, 0, 0, 0, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, Data);
		}

		glGenerateTextureMipmap(TextureHandle);

		Texture->UnloadRawTextureData(0);
	}else if(RenderTarget){
		if(Width == 0 || Height == 0)
			return;

		glCreateFramebuffers(1, &FBO);
		glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		glTextureStorage2D(TextureHandle, 1, RenDev->Use16bit ? GL_RGB565 : GL_RGB8, Width, Height);
		glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, TextureHandle, 0);
		glCreateRenderbuffers(1, &DepthStencilAttachment);
		glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, Width, Height);
		glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilAttachment);

		checkSlow(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	Revision = BaseTexture->GetRevision();
}

void FOpenGLTexture::Free(){
	Width = 0;
	Height = 0;

	if(TextureHandle){
		glDeleteTextures(1, &TextureHandle);
		TextureHandle = GL_NONE;
	}

	if(FBO){
		glDeleteFramebuffers(1, &FBO);
		FBO = GL_NONE;
	}

	if(DepthStencilAttachment){
		glDeleteRenderbuffers(1, &DepthStencilAttachment);
		DepthStencilAttachment = GL_NONE;
	}
}

void FOpenGLTexture::BindTexture(GLuint TextureUnit){
	glBindTextureUnit(TextureUnit, TextureHandle);
}

void FOpenGLTexture::BindRenderTarget(){
	checkSlow(FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void* FOpenGLTexture::ConvertTextureData(FTexture* Texture, ETextureFormat DestFormat, INT Width, INT Height, INT MipIndex){
	void* Result;
	ETextureFormat SrcFormat = Texture->GetFormat();
	INT NumPixels = Width * Height;
	INT SrcBufferSize = GetBytesPerPixel(SrcFormat, NumPixels);
	INT DestBufferSize = GetBytesPerPixel(DestFormat, NumPixels);
	void* TextureData = Texture->GetRawTextureData(MipIndex);

	if(TextureData){
		Result = RenDev->GetScratchBuffer(DestBufferSize);
	}else{
		TextureData = RenDev->GetScratchBuffer(SrcBufferSize + DestBufferSize);
		Result = static_cast<BYTE*>(TextureData) + SrcBufferSize;
		Texture->GetTextureData(MipIndex, TextureData, 0, SrcFormat);
	}

	if(SrcFormat == DestFormat)
		return TextureData;

	if(DestFormat == TEXF_RGBA8){
		if(SrcFormat == TEXF_P8){
			UTexture* Tex = Texture->GetUTexture();
			check(Tex && Tex->Palette);
			const TArray<FColor>& Palette = Tex->Palette->Colors;

			for(INT i = 0; i < NumPixels; ++i)
				static_cast<FColor*>(Result)[i] = Palette[static_cast<BYTE*>(TextureData)[i]];
		}else if(SrcFormat == TEXF_RGB8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE* RGB = static_cast<BYTE*>(TextureData) + i * 3;
				static_cast<FColor*>(Result)[i] = FColor(RGB[0], RGB[1], RGB[2]);
			}
		}else if(SrcFormat == TEXF_L8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Value = static_cast<BYTE*>(TextureData)[i];
				static_cast<FColor*>(Result)[i] = FColor(Value, Value, Value);
			}
		}else if(SrcFormat == TEXF_G16){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Intensity = static_cast<_WORD*>(TextureData)[i] >> 8;
				static_cast<FColor*>(Result)[i] = FColor(Intensity, Intensity, Intensity);
			}
		}else if(SrcFormat == TEXF_V8U8){
			ConvertV8U8ToRGBA8(TextureData, Result, Width, Height);
		}else if(SrcFormat == TEXF_L6V5U5){
			ConvertL6V5U5ToRGBA8(TextureData, Result, Width, Height);
		}else if(SrcFormat == TEXF_X8L8V8U8){
			ConvertX8L8V8U8ToRGB8(TextureData, Result, Width, Height);
		}else{
			Result = NULL;
		}
	}else{
		Result = NULL;
	}

	if(!Result)
		appErrorf("Invalid texture format conversion (%i -> %i)", SrcFormat, DestFormat);

	return Result;
}
