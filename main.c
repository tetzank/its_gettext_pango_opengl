#include <stdio.h>
#include <string.h>

// gettext
#include <libintl.h>
#include <locale.h>

// libxml2
#include <libxml/parser.h>
#include <libxml/tree.h>

// pango using cairo as backend
#include <pango/pangocairo.h>

//#include <GL/gl.h>
#include <GL/glut.h>

#define WIDTH 800
#define HEIGHT 600

GLuint tex_id;


// search recursively for History-tag and give back content of text-node in it
char *traverseXML(xmlNode *n){
	xmlNode *curNode;
	char *res;
	// loop over siblings
	for(curNode=n; curNode; curNode=curNode->next){
		if(strcmp("History", curNode->name) == 0){
			return curNode->children->content;
		}else{
			// go one level deeper
			res = traverseXML(curNode->children);
			if(res) return res;
		}
	}
	return NULL;
}

void rendertext(cairo_t *cr, const char *text){
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr); // fill background with set color

	// flip image because opengl has origin in bottom left, not top left
	cairo_translate(cr, 0.0, HEIGHT);
	cairo_scale(cr, 1.0, -1.0);

	// move it a bit from the edges
	cairo_translate(cr, 10, 20);
	// set text color
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

	// now stage clear for pango
	PangoLayout *layout = pango_cairo_create_layout(cr);
	// set width to show wrapping
	pango_layout_set_width(layout, 400*PANGO_SCALE);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
	// throw text at it
	pango_layout_set_text(layout, text, -1);

	// specify font
	PangoFontDescription *desc = pango_font_description_from_string("Sans Bold 12");
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	// now draw
	pango_cairo_show_layout(cr, layout);
	// free
	g_object_unref(layout);
}

void render(){
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, tex_id);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
	glEnd();

	glutSwapBuffers();
}

void onExit(){
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex_id);
}

int main(int argc, char *argv[]){
	// setup window and OpenGL context with glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("its_gettext_pango_opengl");
	glutDisplayFunc(render);

	// setup gettext to use po subdirectory instead of std /usr/share/locale
	setlocale(LC_ALL, "");
	bindtextdomain("hoplite", "po");
	textdomain("hoplite"); // set std domain -> use hoplite.mo

	// init xml parser
	xmlDoc *doc = xmlReadFile("hele_infantry_spearman_b.xml", NULL, 0);
	if(!doc){
		perror("could not parse data xml");
		return 1;
	}
	xmlNode *root = xmlDocGetRootElement(doc);

	// cairo stuff
	int channels = 4;
	unsigned char *surf_data = calloc(channels * WIDTH * HEIGHT, sizeof(unsigned char));
	cairo_surface_t *surf = cairo_image_surface_create_for_data(
		surf_data,
		CAIRO_FORMAT_ARGB32,
		WIDTH,
		HEIGHT,
		channels * WIDTH
	);
	cairo_t *cr = cairo_create(surf);

	// draw text with pango
	rendertext(
		cr,
		gettext(traverseXML(root)) // translate History tag
	);

	// done drawing
	cairo_destroy(cr);

	// OpenGL stuff
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image into texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		WIDTH,
		HEIGHT,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		surf_data
	);
	// delete cairo stuff as we have our texture
	cairo_surface_destroy(surf);
	free(surf_data);

	// cleanup xml parser
	xmlFreeDoc(doc);
	xmlCleanupParser();

	atexit(onExit);
	glutMainLoop();

	return 0;
}
