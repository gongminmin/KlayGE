<?xml version="1.0" encoding="gb2312" ?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
	<xsl:template match="/">
		<html>
		<head>
			<title><xsl:value-of select="说明/标题"/></title>
			<meta http-equiv="Content-Type" content="text/html; charset=gb2312"/>
		</head>
		<body style="line-height: 150%">
			<xsl:apply-templates select="说明/标题"/>
			<h2>描述：</h2>
			<xsl:apply-templates select="说明/描述"/>
			<hr/>
			<h2>成员：</h2>
			<table border="1" width="100%">
				<xsl:for-each select="说明/函数">
					<tr>
						<td style="line-height: 150%">
							<a><xsl:attribute name="href"><xsl:value-of select="锚点"/></xsl:attribute><xsl:value-of select="名称"/></a>
						</td>
						<td style="line-height: 150%">
							<xsl:value-of select="描述"/>
						</td>
					</tr>
				</xsl:for-each>
			</table>
			<hr/>
			<h2>函数：</h2>
			<xsl:apply-templates select="说明/函数"/>
			<hr/>
			<div style="text-align: center">
				<a href="http://www.enginedev.com" target="_blank">粘土游戏引擎</a><br/>
				版权所有(C) 北京工商大学 计算机学院 计算机003班 龚敏敏，2003
			</div>
		</body>
		</html>
	</xsl:template>

	<xsl:template match="标题">
		<h1 style="text-align: center"><xsl:value-of/></h1>
	</xsl:template>

	<xsl:template match="描述">
		<p><xsl:value-of/></p>
	</xsl:template>

	<xsl:template match="函数">
		<hr/>
		<a><xsl:attribute name="name"><xsl:value-of select="名称"/></xsl:attribute>原型：</a>
		<xsl:apply-templates select="原型"/>
		<xsl:apply-templates select="描述"/>
		<xsl:apply-templates select="返回"/>
		<xsl:apply-templates select="参数描述"/>
		<xsl:apply-templates select="注意"/>
	</xsl:template>

	<xsl:template match="原型">
		<blockquote>
			<pre style="background-color: #C0C0C0; color: #000000; font-weight: bold; padding: 5">
				<xsl:value-of/>
			</pre>
		</blockquote>
	</xsl:template>

	<xsl:template match="描述">
		<xsl:value-of/>
	</xsl:template>

	<xsl:template match="返回">
		<h4>返回：</h4>
		<xsl:value-of/>
	</xsl:template>

	<xsl:template match="参数描述">
		<dl>
			<h4>参数：</h4>
			<xsl:apply-templates select="参数"/>
		</dl>
	</xsl:template>

	<xsl:template match="参数">
		<dt style="font-style: italic">[<xsl:value-of select="类型"/>] <xsl:value-of select="名称"/></dt>
		<dd><xsl:value-of select="描述"/></dd>
	</xsl:template>

	<xsl:template match="注意">
		<h4>注意：</h4>
		<xsl:value-of/>
	</xsl:template>
</xsl:stylesheet>