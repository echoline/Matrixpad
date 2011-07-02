#include "Matrix.h"
#include "Matrixpad.ui.h"
#include <gtk/gtk.h>
#include <cerrno>
#include <sys/time.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#define gtk_statusbar(STR) \
	gtk_statusbar_pop(status_bar, status_bar_ctx); \
	gtk_statusbar_push(status_bar, status_bar_ctx, STR);
#define gtk_text_view_set_text(WIDGET, BUFFER) \
	gtk_text_buffer_set_text(txtbuf, BUFFER, -1); \
	gtk_text_view_set_buffer(WIDGET, txtbuf);
using namespace std;
GtkBuilder	*builder;
GtkWidget	*window;
GtkWidget	*new_window;
GtkWidget	*precision_window;
GtkTextBuffer	*txtbuf;
GtkStatusbar	*status_bar;
guint		status_bar_ctx = -1;
int		precision = 1000;
vector<Matrix>	stack;

double operator- (struct timeval &l, struct timeval &r) {
	double ret = l.tv_sec + (l.tv_usec / 1000000.0);
	ret -= r.tv_sec + (r.tv_usec / 1000000.0);
	return ret;
}

extern "C" void on_about_clicked(GtkWidget *button, gpointer unused) {
	gtk_show_about_dialog(GTK_WINDOW(window), 
		"comments",
		"Eli Cohen",
		NULL);
}

extern "C" void on_open_clicked(GtkWidget *button, gpointer unused) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", (GtkWindow*)window,
                                                GTK_FILE_CHOOSER_ACTION_OPEN,
                                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                NULL);
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *new_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_statusbar("Opening...");
		ifstream ifs(new_filename);
		char buf[8192];
		if (!ifs.good() || !ifs.get(buf, 8192, '\0')) {
			gtk_statusbar("Opening... failed!");
			gtk_widget_destroy(dialog);
			return;
		}
		GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
		gtk_text_view_set_text((GtkTextView*)widget, buf);
		ifs.close();
		gtk_statusbar("Opened");
	}
	gtk_widget_destroy (dialog);
}

extern "C" void on_save_clicked(GtkWidget *button, gpointer unused) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File", (GtkWindow*)window,
                                                GTK_FILE_CHOOSER_ACTION_SAVE,
                                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                NULL);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), ".");

	GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(txtbuf, &start);
	gtk_text_buffer_get_end_iter(txtbuf, &end);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *new_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		FILE *f = fopen(new_filename, "w");
		if (!f) {
			gtk_statusbar("Saving... failed!");
			gtk_widget_destroy(dialog);
			return;
		}
		fprintf(f, "%s", gtk_text_buffer_get_text(txtbuf, &start, &end, true));
		fclose(f);
		gtk_statusbar("Saved");
	}
	gtk_widget_destroy (dialog);
}

Matrix read_matrix() {
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(txtbuf, &start);
	gtk_text_buffer_get_end_iter(txtbuf, &end);
	string str = gtk_text_buffer_get_text(txtbuf, &start, &end, true);
	stringstream ss(str);
	char buf[1024];
	int rows = 0, columns = 0, i;
	double temp;
	bool firstrow = true;
	vector<double> v;
	while (!ss.eof()) {
		ss.getline(buf, 1024);
		stringstream ssi(buf);
		i = 0;
		while (ssi >> temp) {
			v.push_back(temp);
			if (firstrow)
				columns++;
			i++;
		}
		firstrow = false;
		if ((i != columns) && (i > 0)) throw "Columns mismatch";
		else if (i > 0) rows++;
	}
	if (v.size() == 0) throw "No matrix?";
	Matrix A(rows, columns);
	for (int r = 0; r < rows; r++)
		for (int c = 0; c < columns; c++)
			A[r][c] = v[r * columns + c];
	return A;
}

void show_matrix(Matrix A) {
	GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, "textview1"));
	stringstream ss;
	A.Precision(precision);
	ss << A;
	gtk_text_view_set_text((GtkTextView*)widget, ss.str().c_str());
}

extern "C" void on_solve_clicked(GtkWidget *button, gpointer unused) {
	struct timeval then, now;
	char buf[1024];
	try {
		gtk_statusbar("Solving...");
		Matrix A = read_matrix();

		gettimeofday(&then, NULL);
		A = A.Gauss();
		gettimeofday(&now, NULL);

		show_matrix(A);
		sprintf(buf, "Solved in %lf seconds.", now - then);
		gtk_statusbar(buf);
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Solving... failed!");
	}
}

extern "C" void on_transpose_clicked(GtkWidget *button, gpointer unused) {
	struct timeval then, now;
	char buf[1024];
	try {
		gtk_statusbar("Finding Transpose...");
		Matrix A = read_matrix();

		gettimeofday(&then, NULL);
		A = A.Trn();
		gettimeofday(&now, NULL);

		show_matrix(A);
		sprintf(buf, "Transpose found in %lf seconds.", now - then);
		gtk_statusbar(buf);
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Finding Transpose... failed!");
	}
}

extern "C" void on_inverse_clicked(GtkWidget *button, gpointer unused) {
	struct timeval then, now;
	char buf[1024];
	try {
		gtk_statusbar("Finding Inverse...");
		Matrix A = read_matrix();

		gettimeofday(&then, NULL);
		A = A.Inv();
		gettimeofday(&now, NULL);

		show_matrix(A);
		sprintf(buf, "Inverse found in %lf seconds.", now - then);
		gtk_statusbar(buf);
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Finding Inverse... failed!");
	}
}

extern "C" void on_new_clicked(GtkWidget *button, gpointer unused) {
	gtk_widget_show_all(new_window);
}

extern "C" void precision_dialog(GtkWidget *button, gpointer unused) {
	GtkSpinButton *widget = (GtkSpinButton*)gtk_builder_get_object(builder, "spinbutton1");
	gtk_spin_button_set_value(widget, precision);
	gtk_spin_button_set_range(widget, 0, 1000);
	gtk_spin_button_set_increments(widget, 1, 10);
	gtk_widget_show_all(precision_window);
}

extern "C" void adjust_precision(GtkWidget *button, gpointer unused) {
	precision = gtk_spin_button_get_value((GtkSpinButton*)button);
}

extern "C" void push_matrix(GtkWidget *button, gpointer unused) {
	try {
		stack.push_back(read_matrix());
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Failed to put matrix on stack!");
	}
}

extern "C" void pop_matrix(GtkWidget *button, gpointer unused) {
	try {
		if (stack.size() == 0) throw "Empty stack";
		show_matrix(stack.back());
		stack.pop_back();
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Failed to get matrix from stack!");
	}
}

extern "C" void multiply(GtkWidget *button, gpointer unused) {
	struct timeval then, now;
	char buf[1024];
	try {
		if (stack.size() < 2) throw "Please push two matrices to stack first.";
		Matrix B = stack.back();
		stack.pop_back();
		Matrix A = stack.back();
		stack.pop_back();

		gettimeofday(&then, NULL);
		Matrix C = A * B;
		gettimeofday(&now, NULL);

		show_matrix(C);
		sprintf(buf, "Matrices multiplied in %lf seconds.", now - then);
		gtk_statusbar(buf);
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Failed to multiply matrices!");
	}
}

extern "C" void create_new_matrix(GtkWidget *button, gpointer unused) {
	GtkEntry *rows = (GtkEntry*)gtk_builder_get_object(builder, "entry1");
	GtkEntry *cols = (GtkEntry*)gtk_builder_get_object(builder, "entry2");
	int r = atoi(gtk_entry_get_text(rows));
	int c = atoi(gtk_entry_get_text(cols));
	mkmatflag flag = ZERO;
	if (gtk_toggle_button_get_active((GtkToggleButton*)gtk_builder_get_object(builder, "radiobutton2"))) {
		flag = ZERO;
	} else if (gtk_toggle_button_get_active((GtkToggleButton*)gtk_builder_get_object(builder, "radiobutton3"))) {
		flag = IDENT;
	} else if (gtk_toggle_button_get_active((GtkToggleButton*)gtk_builder_get_object(builder, "radiobutton4"))) {
		flag = MIXED;
	} else if (gtk_toggle_button_get_active((GtkToggleButton*)gtk_builder_get_object(builder, "radiobutton5"))) {
		flag = RAND;
	} else if (gtk_toggle_button_get_active((GtkToggleButton*)gtk_builder_get_object(builder, "radiobutton6"))) {
		flag = ONE;
	}
	try {
		Matrix A(r, c, flag);
		show_matrix(A);
		gtk_statusbar("Successfully generated matrix.");
	} catch(const char *s) {
		gtk_statusbar(s);
	} catch (...) {
		gtk_statusbar("Failed to generate matrix!");
	}

	gtk_widget_hide_all(new_window);
}

int main(int argc, char *argv[]) {
	gtk_init (&argc, &argv);

	builder = gtk_builder_new();
	gtk_builder_add_from_string(builder, Matrixpad_ui, -1, NULL);
 
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
	new_window = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
	precision_window = GTK_WIDGET(gtk_builder_get_object(builder, "window3"));
	gtk_window_set_icon_name(GTK_WINDOW(window), "gtk-dialog-authentication");
	txtbuf = gtk_text_view_get_buffer((GtkTextView*)gtk_builder_get_object(builder, "textview1"));
	status_bar = GTK_STATUSBAR(gtk_builder_get_object(builder, "statusbar1"));
	status_bar_ctx = gtk_statusbar_get_context_id(status_bar, "statusbar1");
	gtk_statusbar_push(status_bar, status_bar_ctx, "");
	gtk_builder_connect_signals(builder, NULL);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}

