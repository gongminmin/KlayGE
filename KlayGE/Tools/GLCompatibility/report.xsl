<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
		<html>
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
			<title>OpenGL Compatibility</title>
		</head>
		<body>
			<h1>OpenGL Compatibility</h1>
			<xsl:for-each select="compatibility">
				<table width="100%">
					<tr>
						<th style="background: gray; color: white">Vendor:</th>
						<td style="border-bottom: 1px solid black"><xsl:value-of select="@vendor"/></td>
					</tr>
					<tr>
						<th style="background: gray; color: white">Renderer:</th>
						<td style="border-bottom: 1px solid black"><xsl:value-of select="@renderer"/></td>
					</tr>
					<tr>
						<th style="background: gray; color: white">Core:</th>
						<td style="border-bottom: 1px solid black"><xsl:value-of select="@core"/></td>
					</tr>
				</table>

				<h2>Details</h2>
				<xsl:for-each select="version">
					<h3>OpenGL <xsl:value-of select="@name"/> potential support rate: <xsl:value-of select="@rate"/>%</h3>
					<table width="100%" style="border-bottom: 1px solid black">
						<xsl:if test="count(supported) > 0">
							<tr>
								<th style="background: green; color: white; text-align: left">Supported</th>
							</tr>
							<tr>
								<td style="padding-left:20pt">
									<xsl:for-each select="supported">
										<xsl:value-of select="@name"/><br />
									</xsl:for-each>
								</td>
							</tr>
						</xsl:if>
						<xsl:if test="count(unsupported) > 0">
							<tr>
								<th style="background: red; color: white; text-align: left">Unsupported</th>
							</tr>
							<tr>
								<td style="padding-left:20pt">
									<xsl:for-each select="unsupported">
										<xsl:value-of select="@name"/><br />
									</xsl:for-each>
								</td>
							</tr>
						</xsl:if>
					</table>
				</xsl:for-each>
			</xsl:for-each>
		</body>
		</html>
	</xsl:template>
</xsl:stylesheet>
