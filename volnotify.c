#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <libnotify/notify.h>

#define SOUND_CARD "default"
#define SOUND_ELEMENT "Master"

enum sound_modus {
	INCREMENT,
	DECREMENT,
	TOGGLE,
};

typedef struct _NotifyNotificationPrivate {
	guint32 id;
	char *appname;
	char summary[1024];
	const char *icon;
}libnotify;

struct snd_mixer {
	long min;
	long max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *el;
};

static int show_notification(libnotify *notify, guint32 id)
{
	NotifyNotification *container;

	notify_init(notify->appname);
	container = notify_notification_new(notify->summary, NULL, notify->icon);
	notify_notification_set_urgency(container, NOTIFY_URGENCY_NORMAL);

	notify = container->priv;
	notify->id = id;

	notify_notification_show(container, NULL);
	g_object_unref(G_OBJECT(container));
	notify_uninit();

	return 0;
}

int getdwmblockspid()
{
	char buf[16];
	FILE *fp = popen("pidof -s dwmblocks", "r");
	fgets(buf, sizeof(buf), fp);
	pid_t pid = strtoul(buf, NULL, 10);
	pclose(fp);
	return pid;
}

static long get_volume(struct snd_mixer *m)
{
	long cur;
	snd_mixer_selem_get_playback_volume(m->el, 0, &cur);

	return 100 *(cur) / m->max + 1;;
}

#define ICON_PATH "/home/marko/.icons/Clarity/scalable/status/"
#define ICON(icon) ICON_PATH icon

static void set_text(libnotify *n, int state, int volume)
{
	sprintf(n->summary, "<b>System Volume: %d</b>", volume);

	if (!state){
		strcat(n->summary, "[off]");
		n->icon = ICON("audio-volume-muted.svg");
		return;
	}

	if (volume >= 90 && volume <= 100)
		n->icon = ICON("audio-volume-high.svg");
	else if(volume <= 89 && volume >= 40)
		n->icon = ICON("audio-volume-medium.svg");
	else if (volume <= 39 && volume >= 1)
		n->icon = ICON("audio-volume-low.svg");
	else if (volume == 0)
		n->icon = ICON("audio-volume-low-zero-panel.svg");

	return;
}

static int snd_mixer_init(struct snd_mixer *m)
{
	snd_mixer_open(&m->handle, 0);
	snd_mixer_attach(m->handle, SOUND_CARD);
	snd_mixer_selem_register(m->handle, NULL, NULL);
	snd_mixer_load(m->handle);

	snd_mixer_selem_id_malloc(&m->sid);
	snd_mixer_selem_id_set_index(m->sid, 0);
	snd_mixer_selem_id_set_name(m->sid, SOUND_ELEMENT);

	m->el = snd_mixer_find_selem(m->handle, m->sid);

	return 0;
}

static int setvolume(struct snd_mixer *m, enum sound_modus action, long step)
{
	libnotify notify;

	notify.appname = "VolNotify";
	notify.id = 191;
	notify.icon = NULL;

	long curent;
	curent = get_volume(m);

	int newvol, toggle = 0;
	snd_mixer_selem_get_playback_switch(m->el, 2, &toggle);
	switch (action){
		case INCREMENT:
			newvol = (curent + step >= 100 && curent + step >=0 ) ? 100 : curent + step;
			snd_mixer_selem_set_playback_volume_all(m->el, newvol * m->max / 100);
			set_text(&notify, toggle, newvol);
			break;
		case DECREMENT:
			newvol = (curent - step <= 100 && curent - step >= 0) ?  curent - step : 0 ;
			snd_mixer_selem_set_playback_volume_all(m->el, newvol * m->max / 100);
			set_text(&notify, toggle, newvol);
			break;
		case TOGGLE:
			if (snd_mixer_selem_set_playback_switch(m->el, 2, (toggle ? 1 : 0) ^ 1) >= 0)
				set_text(&notify, (toggle ? 1 : 0) ^ 1, curent);
			break;
	}

	show_notification(&notify, notify.id);
	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	long vol;

	struct snd_mixer mixer;
	snd_mixer_init(&mixer);
	snd_mixer_selem_get_playback_volume_range(mixer.el, &mixer.min, &mixer.max);

	for (i = 1; i < argc; i++) {
		switch(argv[i][1]) {
		case 'i':
			vol = atoi(argv[i + 1]);
			setvolume(&mixer, INCREMENT, vol);

			kill(getdwmblockspid(), 44);
			break;
		case 'd':
			vol = atoi(argv[i + 1]);
			setvolume(&mixer, DECREMENT, vol);
			kill(getdwmblockspid(), 44);
			break;
		case 't':
			setvolume(&mixer, TOGGLE, 0);
			kill(getdwmblockspid(), 44);
			break;
		case 'g':
			fprintf(stdout, "%zu", get_volume(&mixer));
			break;
		default:
			break;
		}
	}

	snd_mixer_selem_id_free(mixer.sid);
	snd_mixer_close(mixer.handle);

	return 0;
}
